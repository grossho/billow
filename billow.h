/*
 * Copyright 2011 Vladimir Yu. Stepanov
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 * 
 *    1. Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright notice, this list
 *       of conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY AUTHOR ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL AUTHOR OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if !defined(HEADER_BILLOW)
#define HEADER_BILLOW

#include <Python.h>

#include "pycompat.h"

typedef union __genlink {
	union __genlink* link[2];
	void* point[2];
} genlink;

#define QCL_init(__x0)							\
	do {								\
		register genlink* __x = (__x0);				\
		__x->link[0] = __x;					\
		__x->link[1] = __x;					\
	} while (0)

#define QCL_merge(__x0, __p0, __z0)					\
	do {								\
		if (!(__p0))						\
			QCL_merge_by0(__x0, __z0);			\
		else							\
			QCL_merge_by1(__x0, __z0);			\
	} while (0)

#define QCL_merge_by0(__x0, __z0) 					\
	do {								\
		register genlink* __x = (__x0);				\
		register genlink* __z = (__z0);				\
		if (__z != __z->link[0]) {				\
			register genlink* __y = __x->link[0];		\
			(__x->link[0] = __z->link[0])->link[1] = __x;	\
			(__y->link[1] = __z->link[1])->link[0] = __y;	\
		}							\
	} while (0)

#define QCL_merge_by1(__x0, __z0) 					\
	do {								\
		register genlink* __x = (__x0);				\
		register genlink* __z = (__z0);				\
		if (__z != __z->link[0]) {			\
			register genlink* __y = __x->link[1];		\
			(__x->link[1] = __z->link[1])->link[0] = __x;	\
			(__y->link[0] = __z->link[0])->link[1] = __y;	\
		}							\
	} while (0)

#define QCL_insert(__x0, __p0, __z0)					\
	do {								\
		if (!(__p0))						\
			QCL_insert_by0(__x0, __z0);			\
		else							\
			QCL_insert_by1(__x0, __z0);			\
	} while (0)

#define QCL_insert_by0(__x0, __z0)					\
	do {								\
		register genlink* __x = (__x0);				\
		register genlink* __y = (__x)->link[0];			\
		register genlink* __z = (__z0);				\
		__z->link[0] = __y;					\
		__z->link[1] = __x;					\
		__x->link[0] = __z;					\
		__y->link[1] = __z;					\
	} while (0)

#define QCL_insert_by1(__x0, __z0)					\
	do {								\
		register genlink* __x = (__x0);				\
		register genlink* __y = __x->link[1];			\
		register genlink* __z = (__z0);				\
		__z->link[0] = __x;					\
		__z->link[1] = __y;					\
		__y->link[0] = __z;					\
		__x->link[1] = __z;					\
	} while (0)

#define QCL_selfremove(__x0)						\
	do {								\
		register genlink* __x = (__x0);				\
		register genlink* __y = __x->link[0];			\
		register genlink* __z = __x->link[1];			\
		__y->link[1] = __z;					\
		__z->link[0] = __y;					\
	} while (0)

#define getstruct_by_fieldptr(__struct,__field,__ptr)			\
	((__struct*)&(((char*)__ptr)[(char*)0 - 			\
				(char*)&((__struct*)0)->__field]))

#define sethead_by_fieldptr(__head,__field,__ptr)			\
	do {								\
		__head = (typeof(__head))&((char*)__ptr)[(char*)0 -	\
				(char*)&((typeof(__head))0)->__field];	\
	} while (0)

#define FLAGS_(__value,__var)		(((__var)&(__value)) == (__value))

#define BILLOW_RINGORDER		0x100

typedef struct __billow_node {
	genlink node_link;
	PyObject* node_item;
	Py_ssize_t node_refs;
} billow_node;


extern PyObject* BillowExc_LockError;
extern PyObject* BillowExc_RDLockError;
extern PyObject* BillowExc_WRLockError;
extern PyObject* BillowExc_LimitReachedError;
extern PyObject* BillowExc_ExpectOneError;
extern PyObject* BillowExc_ConsistentError;

#define Py_Nil (&BillowNil)
extern PyObject BillowNil;

extern PyTypeObject BillowNil_Type;
extern PyTypeObject BillowPair_Type;
extern PyTypeObject BillowPairIter_Type;
extern PyObject* zippair(PyObject* self, PyObject* args);

#endif /* HEADER_BILLOW */
