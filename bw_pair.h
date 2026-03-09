/*
 * (c) Vladimir Yu. Stepanov 2006-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(HEADER_BILLOW_PAIR)
#define HEADER_BILLOW_PAIR

#include <Python.h>
#include "billow.h"

typedef struct __bw_pair {
	PyObject_HEAD
	PyObject* pair_o[2];
} bw_pair_t;

extern PyObject* bw_pair_New(PyObject* k, PyObject* v);
extern PyObject* bw_pair_Borrowed(PyObject* k, PyObject* v);

extern PyObject* bw_zippair(PyObject* self, PyObject* args);

extern PyTypeObject bw_pair_Type;
extern PyTypeObject bw_pair_iter_Type;

#endif /* HEADER_BILLOW_PAIR */
