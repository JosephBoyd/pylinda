/*
*    Copyright 2006 Andrew Wilkinson <aw@cs.york.ac.uk>.
*
*    This file is part of LibLinda (http://www-users.cs.york.ac.uk/~aw/pylinda)
*
*    LibLinda is free software; you can redistribute it and/or modify
*    it under the terms of the GNU Lesser General Public License as published by
*    the Free Software Foundation; either version 2.1 of the License, or
*    (at your option) any later version.
*
*    LibLinda is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Lesser General Public License for more details.
*
*    You should have received a copy of the GNU Lesser General Public License
*    along with LibLinda; if not, write to the Free Software
*    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <sys/un.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#define LINDA_SERVER
#include "linda.h"
#include "linda_internal.h"

int Linda_sd;
#ifdef USE_DOMAIN_SOCKETS
int Linda_udd = 0;
#endif
int Linda_port = 2102;
int Linda_active_connections;

unsigned char Linda_is_server = 1;

char* Linda_process_id = NULL;
char* Linda_thread_id = NULL;

#ifdef REGISTER_TYPES
unsigned char Linda_register_types = 1;
#else
unsigned char Linda_register_types = 0;
#endif

unsigned char Linda_inited = 0;

LindaValue Linda_uts;
LindaValue Linda_typeType = NULL;
LindaValue Linda_nilType;
LindaValue Linda_boolType;
LindaValue Linda_intType;
LindaValue Linda_floatType;
LindaValue Linda_stringType;
LindaValue Linda_tupleSpaceType;

void Linda_init() {
    if(Linda_inited) { return; }
    Linda_inited = 1;

    Minimal_init();

#ifdef TYPES
    Linda_typeType = Linda_type("typetype :: type;");
    Linda_typeType->typeobj = Linda_typeType;
    Linda_nilType = Linda_type("niltype :: Nil;");
    Linda_boolType = Linda_type("booltype :: bool;");
    Linda_intType = Linda_type("inttype :: int;");
    Linda_floatType = Linda_type("floattype :: float;");
    Linda_stringType = Linda_type("stringtype :: string;");
    Linda_tupleSpaceType = Linda_type("tupleSpacetype :: tuplespace;");
#endif

    Linda_uts = Minimal_tupleSpace("UTS");
}

unsigned char Linda_serve(unsigned char use_domain, int port) {
    int err;

    Linda_init();

#ifdef USE_DOMAIN_SOCKETS
    if(use_domain) {
    Linda_udd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(Linda_udd != -1) {
        struct sockaddr_un addr_in;
        addr_in.sun_family = AF_UNIX;
        memcpy(addr_in.sun_path, "/tmp/pylinda", 13);

        err = bind(Linda_udd, (struct sockaddr*)&addr_in, sizeof(struct sockaddr));
        if(err == -1) {
            unlink("/tmp/pylinda");
            Linda_udd = 0;
        } else {
            err = listen(Linda_udd, 5);
            if(err == -1) {
                unlink("/tmp/pylinda");
                Linda_udd = 0;
            } else {
                Linda_active_connections += 1;
            }
        }
    }
    } else {
        Linda_udd = 0;
    }
#endif
    Linda_sd = socket(PF_INET, SOCK_STREAM, 0);
    if(Linda_sd == -1) return 0;

    struct sockaddr_in addr_in;
    addr_in.sin_family = AF_INET;
    Linda_port = port;
    addr_in.sin_port = htons(port);
    if(inet_aton("0.0.0.0", (struct in_addr*)&(addr_in.sin_addr.s_addr)) == 0) return 0;

    memset(&(addr_in.sin_zero), 0, 8);

    err = bind(Linda_sd, (struct sockaddr*)&addr_in, sizeof(struct sockaddr));
    if(err == -1) return 0;
    err = listen(Linda_sd, 5);
    if(err == -1) return 0;

    Linda_active_connections += 1;

#ifdef USE_DOMAIN_SOCKETS
    if(Linda_udd == 0 || Linda_sd == 0) {
#else
    if(Linda_sd == 0) {
#endif
        return 0;
    } else {
        sleep(1); /* It appears that if we accept too soon things fail, so pause. */
        return 1;
    }
}

int Linda_accept(int sd) {
    int s = accept(sd, NULL, NULL);

    Linda_init();

    if(s == -1) { fprintf(stderr, "Error in accept(%i): %s\n", sd, strerror(errno)); }
    return s;
}

int Linda_server_disconnect() {
#ifdef USE_DOMAIN_SOCKETS
    shutdown(Linda_udd, SHUT_RDWR);
    unlink("/tmp/pylinda");
    Linda_active_connections -= 1;
#endif
    shutdown(Linda_sd, SHUT_RDWR);
    Linda_active_connections -= 1;
    return 1;
}

int Linda_connect(char* address) {
    int sd;
    int colon;
    int port;
    for(colon=0; colon<strlen(address); colon++) {
        if(address[colon] == ':') {
            break;
        }
    }
    if(colon == strlen(address)) {
        port = 2102;
    } else {
        address[colon] = '\0';
        port = atoi(&(address[colon+1]));
    }

    sd = socket(PF_INET, SOCK_STREAM, 0);
    if(sd == -1) { return 0; }

    struct sockaddr_in addr_in;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(port);
    if(inet_aton(address, (struct in_addr*)&(addr_in.sin_addr.s_addr)) == 0) {
        struct hostent* host = gethostbyname(address);
        if(host == NULL) { fprintf(stderr, "Unable to get hostname: %s\n", strerror(errno)); return 0; }
        memcpy(&(addr_in.sin_addr.s_addr), host->h_addr, host->h_length);
    }

    memset(&(addr_in.sin_zero), 0, 8);

    int err = connect(sd, (struct sockaddr*)&addr_in, sizeof(struct sockaddr));
    if(err == -1) { fprintf(stderr, "Error connecting to %s: %s\n", address, strerror(errno)); return 0; }

    Linda_active_connections += 1;

    return sd;
}

void Linda_disconnect(int sd) {
    shutdown(sd, SHUT_RDWR);
    Linda_active_connections -= 1;
}

void Linda_setNodeID(char* nid) {
    Linda_process_id = malloc(strlen(nid)+1);
    strcpy(Linda_process_id, nid);
}
