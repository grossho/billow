/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(HEADER_BILLOW_NIL)
#define HEADER_BILLOW_NIL

#include <Python.h>

extern PyObject* bw_expectone_at(PyObject* self, PyObject* arg);
extern PyObject* bw_coalesce_at(PyObject* self, PyObject* arg);
extern PyObject* bw_min_at(PyObject* self, PyObject* arg);
extern PyObject* bw_min_override_at(PyObject* self, PyObject* arg);
extern PyObject* bw_max_at(PyObject* self, PyObject* arg);
extern PyObject* bw_max_override_at(PyObject* self, PyObject* arg);
extern PyObject* bw_minmax_at(PyObject* self, PyObject* arg);
extern PyObject* bw_minmax_override_at(PyObject* self, PyObject* arg);
extern PyObject* bw_count_at(PyObject* self, PyObject* arg);

extern PyObject bw_NilStruct;
#define Py_Nil (&bw_NilStruct)

extern PyTypeObject bw_nil_Type;

#endif /* HEADER_BILLOW_NIL */
