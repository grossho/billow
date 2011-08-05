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

#include <Python.h>
#include "pycompat.h"
#include "pyexceptionlist.h"

int
exceptionlist__initmodule(PyObject* module, struct exceptionlist* exclist) {
	const struct exceptionlist* exc;
	char* longname = NULL;
	PyObject* name;
	Py_ssize_t size;
	Py_ssize_t maxsize;

	if ((name = PyObject_GetAttrString(module, "__name__")) == NULL)
		return -1;
	if (!PyString_Check(name)) {
		PyErr_SetString(PyExc_TypeError, "__name__ required an str()");
		goto failed;
	}
	size = PyString_GET_SIZE(name);
	for (maxsize = 0, exc = exclist; exc->exc_name != NULL; exc++) {
		register int i = strlen(exc->exc_name);
		if (i > maxsize)
			maxsize = i;
	}
	if ((longname = PyMem_MALLOC(size + sizeof(".") + maxsize + sizeof("\0"))) == NULL) {
		PyErr_NoMemory();
		goto failed;
	}
	memcpy(longname, PyString_AS_STRING(name), size);
	Py_DECREF(name);
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
	Py_DECREF(name);
	return -1;
}
