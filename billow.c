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
#include "billow.h"
#include "billow_nil.h"

PyObject* BillowExc_ExpectOneError;
PyObject* BillowExc_ConsistentError;
PyObject* BillowExc_LimitReachedError;
PyObject* BillowExc_LockError;
PyObject* BillowExc_RDLockError;
PyObject* BillowExc_WRLockError;

struct exceptionlist billow_exceptionlist[] = {
{ "ExpectOneError", &BillowExc_ExpectOneError, &PyExc_StandardError, "expect only one value" },
{ "ConsistentError", &BillowExc_ConsistentError, &PyExc_StandardError, "" },
{ "LimitReachedError", &BillowExc_LimitReachedError, &PyExc_StandardError, "" },
{ "LockError", &BillowExc_LockError, &PyExc_StandardError, "" },
{ "RDLockError", &BillowExc_RDLockError, &BillowExc_LockError, "" },
{ "WRLockError", &BillowExc_WRLockError, &BillowExc_LockError, "" },
{ NULL }
};

void
register_into_builtin(void) {
	PyObject* builtin;

	if ((builtin = PyImport_ImportModule("__builtin__")) == NULL)
		return;

	Py_INCREF(&BillowNil);
	if (PyModule_AddObject(builtin, "Nil", (PyObject*)&BillowNil) < 0)
		return;

	Py_INCREF(&BillowPair_Type);
	if (PyModule_AddObject(builtin, "pair", (PyObject*)&BillowPair_Type) < 0)
		return;
}


static PyMethodDef module_Methods[] = {
{ "maxowe_it", (PyCFunction)billow_maxowe, METH_O, NULL },
{ "minowe_it", (PyCFunction)billow_minowe, METH_O, NULL },
{ "edgeowe_it", (PyCFunction)billow_edgeowe, METH_O, NULL },
{ "expectone_it", (PyCFunction)billow_expectone, METH_O, NULL },
{ "coalesce_it", (PyCFunction)billow_coalesce, METH_O, NULL },
{ "maxowe", (PyCFunction)billow_maxowe, METH_VARARGS, NULL },
{ "minowe", (PyCFunction)billow_minowe, METH_VARARGS, NULL },
{ "edgeowe", (PyCFunction)billow_edgeowe, METH_VARARGS, NULL },
{ "expectone", (PyCFunction)billow_expectone, METH_VARARGS, NULL },
{ "coalesce", (PyCFunction)billow_coalesce, METH_VARARGS, NULL },
{ NULL, NULL, 0, NULL }
};

PyMODINIT_FUNC
initbillow(void) {
	PyObject* billow;
	PyObject* builtin;

	if ((builtin = PyImport_ImportModule("__builtin__")) == NULL)
		return;

	if ((billow = Py_InitModule4("billow", module_Methods, "billow", NULL, PYTHON_API_VERSION)) == NULL)
		return;

	if (exceptionlist__initmodule(billow, billow_exceptionlist))
		return;

	if (PyType_Ready(&BillowNil_Type) < 0)
		return;

	if (PyType_Ready(&BillowPair_Type) < 0)
		return;

	if (PyType_Ready(&BillowPairIter_Type) < 0)
		return;

	Py_INCREF(&BillowNil);
	if (PyModule_AddObject(billow, "Nil", (PyObject*)&BillowNil) < 0)
		return;

	Py_INCREF(&BillowPair_Type);
	if (PyModule_AddObject(billow, "pair", (PyObject*)&BillowPair_Type) < 0)
		return;
}
