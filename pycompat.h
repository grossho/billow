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

#if !defined(HEADER_PYCOMPAT)
#define HEADER_PYCOMPAT

#include <Python.h>

#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)

typedef int Py_ssize_t;

#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN

#define PyInt_FromSsize_t(x) PyInt_FromLong(x)
#define PyIndex_Check(x) PyInt_Check(x)
#define PyNumber_AsSsize_t(x,e) (PyInt_Check(x) ? PyInt_AS_LONG(x) : (PyErr_SetNone(e), -1))

typedef Py_ssize_t (*lenfunc)(PyObject *);
typedef PyObject *(*ssizeargfunc)(PyObject *, Py_ssize_t);
typedef PyObject *(*ssizessizeargfunc)(PyObject *, Py_ssize_t, Py_ssize_t);
typedef int(*ssizessizeobjargproc)(PyObject *, Py_ssize_t, Py_ssize_t, PyObject *);

#endif

#if PY_VERSION_HEX < 0x02050000
#define PyObject_LENGTH_HINT(x) PyObject_Size(x)
#elif PY_VERSION_HEX < 0x02060000
#define PyObject_LENGTH_HINT(x) _PyObject_LengthHint(x)
#else
#define PyObject_LENGTH_HINT(x) _PyObject_LengthHint(x, -1)
#endif

#if PY_VERSION_HEX < 0x02060100
#define Py_TYPE(ob) (((PyObject*)(ob))->ob_type)
#endif

#if PY_VERSION_HEX < 0x02050000
#define Py_ssize_t_FROM_FORMAT	"%d"
#define Py_ssize_t_PARSE_FORMAT	"i"
#else
#define Py_ssize_t_FROM_FORMAT	"%zd"
#define Py_ssize_t_PARSE_FORMAT	"n"
#endif

#endif /* HEADER_PYCOMPAT */
