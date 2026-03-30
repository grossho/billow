/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(HEADER_EXCEPTIONLIST)
#define HEADER_EXCEPTIONLIST

#include <Python.h>

struct exceptionlist {
	const char* exc_name;
	PyObject** exc_this;
	PyObject** exc_base;
	const char* exc_doc;
};

extern int exceptions_into_module(PyObject* module, struct exceptionlist* exclist, const char* modulename);

#endif /* HEADER_EXCEPTIONLIST */
