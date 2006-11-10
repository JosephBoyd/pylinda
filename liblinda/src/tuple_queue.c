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

#include <stdlib.h>

#include "linda.h"
#include "linda_internal.h"

Tuplequeue Tuplequeue_push(Tuplequeue tq) {
    Tuplequeue ntq = malloc(sizeof(struct Tuplequeue_t));
    ntq->tuple = Linda_tuple(0);
    ntq->next = tq;
    return ntq;
}

Tuplequeue Tuplequeue_pop(Tuplequeue tq) {
    Tuplequeue ntq = tq->next;
    Linda_delReference(tq->tuple);
    free(tq);
    return ntq;
}

LindaValue Tuplequeue_top(Tuplequeue tq) {
    return tq->tuple;
}
