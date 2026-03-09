/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#include <Python.h>
#include "pycompat.h"
#include "pyexceptionlist.h"

int
exceptions_into_module(PyObject* module, struct exceptionlist* exclist, const char* modulename) {
	const struct exceptionlist* exc;
	char* longname = NULL;
	Py_ssize_t size = strlen(modulename);
	Py_ssize_t maxsize;

	for (maxsize = 0, exc = exclist; exc->exc_name != NULL; exc++) {
		int i = strlen(exc->exc_name);
		if (i > maxsize)
			maxsize = i;
	}
	if ((longname = PyMem_MALLOC(size + sizeof(".") + maxsize + sizeof("\0"))) == NULL) {
		PyErr_NoMemory();
		goto failed;
	}
	memcpy(longname, modulename, size);
	longname[size++] = '.';
	for (exc = exclist; exc->exc_name != NULL; exc++) {
		PyObject* dict;
		PyObject* s;
		
		if ((dict = PyDict_New()) == NULL)
			goto failed;
		if ((s = PyString_FromString(exc->exc_doc)) == NULL) {
			Py_DECREF(dict);
			goto failed;
		}
		if (PyDict_SetItemString(dict, "__doc__", s) < 0) {
			Py_DECREF(dict);
			Py_DECREF(s);
			goto failed;
		}
		Py_DECREF(s);
		strcpy(&longname[size], exc->exc_name);
		if (*exc->exc_this == NULL &&
		   (*exc->exc_this = PyErr_NewException(longname, *exc->exc_base, dict)) == NULL) {
			Py_DECREF(dict);
			goto failed;
		}
		Py_DECREF(dict);
	}
	PyMem_FREE(longname);
	for (exc = exclist; exc->exc_name != NULL; exc++) {
		Py_INCREF(*exc->exc_this);
		if (PyModule_AddObject(module, exc->exc_name, *exc->exc_this) < 0)
			return -1;
	}
	return 0;
failed:
	if (longname != NULL)
		PyMem_FREE(longname);
	return -1;
}
