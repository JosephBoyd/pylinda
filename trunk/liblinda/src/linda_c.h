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

#include "minimal.h"

#ifndef __LINDA_C_H__
#define __LINDA_C_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef MinimalValue LindaValue;

static inline unsigned char Linda_isNil(LindaValue v) { return Minimal_isNil(v); }
static inline LindaValue Linda_nil() { return Minimal_nil(); }

static inline unsigned char Linda_isBool(LindaValue v) { return Minimal_isBool(v); }
static inline LindaValue Linda_bool(unsigned char b) { return Minimal_bool(b); }
static inline unsigned char Linda_getBool(LindaValue v) { return Minimal_getBool(v); }

static inline unsigned char Linda_isInt(LindaValue v) { return Minimal_isInt(v); }
static inline LindaValue Linda_int(int i) { return Minimal_int(i); }
static inline int Linda_getInt(LindaValue v) { return Minimal_getInt(v); }

static inline unsigned char Linda_isFloat(LindaValue v) { return Minimal_isFloat(v); }
static inline LindaValue Linda_float(float f) { return Minimal_float(f); }
static inline float Linda_getFloat(LindaValue v) { return Minimal_getFloat(v); }

static inline unsigned char Linda_isTypeSpec(LindaValue v) { return Minimal_isFloat(v); }
static inline LindaValue Linda_type(const char* typespec) { return Minimal_type(typespec); }

static inline unsigned char Linda_isString(LindaValue v) { return Minimal_isString(v); }
static inline LindaValue Linda_string(char* s) { return Minimal_string(s); }
static inline LindaValue Linda_string2(char* s, int len) { return Minimal_string2(s, len); }
static inline char* Linda_getString(LindaValue v) { return Minimal_getString(v); }
static inline int Linda_getStringLen(LindaValue v) { return Minimal_getStringLen(v); }

static inline unsigned char Linda_isTupleSpace(LindaValue v) { return Minimal_isTupleSpace(v); }
static inline LindaValue Linda_tupleSpace(const char* ts) { return Minimal_tupleSpace(ts); }
static inline char* Linda_getTupleSpace(LindaValue v) { return Minimal_getTupleSpace(v); }

static inline unsigned char Linda_isTuple(LindaValue v) { return Minimal_isTuple(v); }
static inline LindaValue Linda_tuple(int i) { return Minimal_tuple(i); }
static inline int Linda_getTupleSize(LindaValue v) { return Minimal_getTupleSize(v); }
static inline void Linda_tupleAdd(LindaValue t, LindaValue v) { Minimal_tupleAdd(t, v); }
static inline void Linda_tupleSet(LindaValue t, int i, LindaValue v) { Minimal_tupleSet(t, i, v); }
static inline LindaValue Linda_tupleGet(LindaValue t, int i) { return Minimal_tupleGet(t, i); }

static inline LindaValue Linda_copy(LindaValue v) { return Minimal_copy(v); }

extern char* version;
extern char* process_id;

#ifndef NULL
#define NULL (void*)0
#endif

#ifndef LINDA_SERVER
unsigned char Linda_connect(int port);
extern int Linda_port;
void Linda_disconnect();
#endif

typedef char* Linda_tuplespace;

Linda_tuplespace Linda_createTuplespace();

void Linda_addTSReference(const Linda_tuplespace ts);
void Linda_delTSReference(const Linda_tuplespace ts);

#define Linda_addReference(obj) Minimal_addReference2(obj, __FILE__, __LINE__);
void Linda_addReference2(MinimalObject ptr, char* file, int line);
int Linda_getReferenceCount(MinimalObject ptr);
void Linda_delReference(MinimalObject ptr);

void Linda_out(const Linda_tuplespace ts, LindaValue t);
LindaValue Linda_in(const Linda_tuplespace ts, LindaValue t);
LindaValue Linda_rd(const Linda_tuplespace ts, LindaValue t);
LindaValue Linda_inp(const Linda_tuplespace ts, LindaValue t);
LindaValue Linda_rdp(const Linda_tuplespace ts, LindaValue t);

int Linda_collect(const Linda_tuplespace ts1, const Linda_tuplespace ts2, LindaValue t);
int Linda_copy_collect(const Linda_tuplespace ts1, const Linda_tuplespace ts2, LindaValue t);

extern Linda_tuplespace Linda_uts;

extern LindaValue Linda_boolType;
extern LindaValue Linda_intType;
extern LindaValue Linda_floatType;
extern LindaValue Linda_stringType;
extern LindaValue Linda_tupleSpaceType;

#ifdef __cplusplus
}
#endif

#endif
