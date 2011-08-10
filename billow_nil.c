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
#include "billow.h"
#include "billow_nil.h"

PyObject BillowNil = { 1, &BillowNil_Type };

static
void
billow_nil_dealloc(PyObject* self) {
}

static
int
billow_nil_false(PyObject* self) {
	return 0;
}

static
PyObject*
billow_nil_selfunary(PyObject* self) {
	self = Py_Nil;
	Py_INCREF(self);
	return self;
}

static
PyObject*
billow_nil_divmod(PyObject* self, PyObject* arg) {
	if ((arg = PyTuple_New(2)) == NULL)
		return NULL;
	self = Py_Nil;
	PyTuple_SET_ITEM(arg, 0, self);
	Py_INCREF(self);
	PyTuple_SET_ITEM(arg, 1, self);
	Py_INCREF(self);
	return arg;
}

static
PyObject*
billow_nil_selfbinary(PyObject* self, PyObject* arg) {
	self = Py_Nil;
	Py_INCREF(self);
	return self;
}

static
PyObject*
billow_nil_selfternary(PyObject* self, PyObject* arg1, PyObject* arg2) {
	self = Py_Nil;
	Py_INCREF(self);
	return self;
}

static
int
billow_nil_coerce(PyObject** pv, PyObject** pw) {
	Py_INCREF(*pv);
	Py_INCREF(*pw);
	return 0;
}

static
PyObject*
billow_nil_richcompare(PyObject* v, PyObject* w, int op) {
	v = Py_Nil;
	Py_INCREF(v);
	return v;
}

static
PyObject*
billow_nil_repr(PyObject* self) {
	return PyString_InternFromString("Nil");
}

static
PyNumberMethods
billow_nil_number = {
	(binaryfunc)billow_nil_selfbinary,	/* nb_add */
	(binaryfunc)billow_nil_selfbinary, 	/* nb_subtract */
	(binaryfunc)billow_nil_selfbinary,	/* nb_multiply */
	(binaryfunc)billow_nil_selfbinary,	/* nb_divide */
	(binaryfunc)billow_nil_selfbinary,	/* nb_remainder */
	(binaryfunc)billow_nil_divmod,		/* nb_divmod */
	(ternaryfunc)billow_nil_selfternary,	/* nb_power */
	(unaryfunc)billow_nil_selfunary,	/* nb_negative */
	(unaryfunc)billow_nil_selfunary,	/* nb_positive */
	(unaryfunc)billow_nil_selfunary,	/* nb_absolute */
	(inquiry)billow_nil_false,		/* nb_nonzero */
	(unaryfunc)billow_nil_selfunary,	/* nb_invert */
	(binaryfunc)billow_nil_selfbinary,	/* nb_lshift */
	(binaryfunc)billow_nil_selfbinary,	/* nb_rshift */
	(binaryfunc)billow_nil_selfbinary,	/* nb_and */
	(binaryfunc)billow_nil_selfbinary,	/* nb_xor */
	(binaryfunc)billow_nil_selfbinary,	/* nb_or */
	(coercion)billow_nil_coerce,		/* nb_coerce */
	0,					/* nb_int */
	0,					/* nb_long */
	0,					/* nb_float */
	0,					/* nb_oct */
	0,					/* nb_hex */
	(binaryfunc)billow_nil_selfbinary,	/* nb_inplace_add */
	(binaryfunc)billow_nil_selfbinary,	/* nb_inplace_subtract */
	(binaryfunc)billow_nil_selfbinary,	/* nb_inplace_multiply */
	(binaryfunc)billow_nil_selfbinary,	/* nb_inplace_divide */
	(binaryfunc)billow_nil_selfternary,	/* nb_inplace_remainder */
	(ternaryfunc)billow_nil_selfternary,	/* nb_inplace_power */
	(binaryfunc)billow_nil_selfbinary,	/* nb_inplace_lshift */
	(binaryfunc)billow_nil_selfbinary,	/* nb_inplace_rshift */
	(binaryfunc)billow_nil_selfbinary,	/* nb_inplace_and */
	(binaryfunc)billow_nil_selfbinary,	/* nb_inplace_xor */
	(binaryfunc)billow_nil_selfbinary,	/* nb_inplace_or */
};

PyTypeObject
BillowNil_Type = {
	PyObject_HEAD_INIT(NULL)
	0,
	"Nil",
	sizeof(PyObject),
	0,
	(destructor)billow_nil_dealloc,		/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	(reprfunc)billow_nil_repr,		/* tp_repr */
	(PyNumberMethods *)&billow_nil_number,	/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	(reprfunc)billow_nil_repr,		/* tp_str */
	(getattrofunc)PyObject_GenericGetAttr,	/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_CHECKTYPES, /* tp_flags */
	"Interoperable Nil object. It is like NULL in SQL statement.", /* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	(richcmpfunc)billow_nil_richcompare,	/* tp_richcompare */
};

PyObject*
billow_expectone(PyObject* self, PyObject* seq) {
	PyObject* list = NULL;
	PyObject* edge = NULL;
	PyObject* next = NULL;
	PyObject* it;

	if ((it = PyObject_GetIter(seq)) == NULL)
		goto failed;
	for (;;) {
		if ((next = PyIter_Next(it)) == NULL) {
			if (PyErr_Occurred())
				goto failed;
			break;
		}
		if (next == Py_Nil) {
			Py_DECREF(next);
			continue;
		} else {
			edge = next;
			for (;;) {
				if ((next = PyIter_Next(it)) == NULL) {
					if (PyErr_Occurred())
						goto failed;
					break;
				}
				if (next == Py_Nil) {
					Py_DECREF(next);
					continue;
				}
				if (list != NULL) {
					if (PyList_Append(list, next) < 0)
						goto failed;
				} else {
					if ((list = PyList_New(2)) == NULL)
						goto failed;
					PyList_SET_ITEM(list, 0, edge);
					PyList_SET_ITEM(list, 1, next);
					edge = NULL;
				}
			}
			if (list != NULL) {
				PyErr_SetObject(BillowExc_ExpectOneError, list);
				goto failed;
			}
		}
	}
	Py_DECREF(it);
	if (edge == NULL) {
		edge = Py_Nil;
		Py_INCREF(edge);
	}
	return edge;
failed:
	Py_XDECREF(list);
	Py_XDECREF(edge);
	Py_XDECREF(next);
	Py_XDECREF(it);
	return NULL;
}

PyObject*
billow_coalesce(PyObject* self, PyObject* seq) {
	PyObject* next;
	PyObject* it;

	if ((it = PyObject_GetIter(seq)) == NULL)
		return NULL;
	for (;;) {
		if ((next = PyIter_Next(it)) == NULL) {
			if (PyErr_Occurred())
				goto failed;
			next = Py_Nil;
			Py_INCREF(next);
			break;
		}
		if (next != Py_Nil)
			break;
		Py_DECREF(next);
	}
failed:
	Py_DECREF(it);
	return next;
}

static
PyObject*
billow_diff(PyObject* seq, int op) {
	PyObject* edge = NULL;
	PyObject* next = NULL;
	PyObject* it;
	int cmp;

	if ((it = PyObject_GetIter(seq)) == NULL)
		goto failed;
	for (;;) {
		if ((edge = PyIter_Next(it)) == NULL) {
			if (PyErr_Occurred())
				goto failed;
			edge = Py_Nil;
			Py_INCREF(edge);
			break;
		}
		if (edge != Py_Nil) {
			for (;;) {
				if ((next = PyIter_Next(it)) == NULL) {
					if (PyErr_Occurred())
						goto failed;
					break;
				}
				if (next == Py_Nil) {
					Py_DECREF(next);
					continue;
				}
				if ((cmp = PyObject_RichCompareBool(next, edge, op)) < 0)
					goto failed;
				if (cmp > 0) {
					Py_DECREF(edge);
					edge = next;
				} else
					Py_DECREF(next);
			}
			break;
		}
		Py_DECREF(edge);
	}
	Py_DECREF(it);
	return edge;
failed:
	Py_XDECREF(edge);
	Py_XDECREF(next);
	Py_XDECREF(it);
	return NULL;
}

PyObject*
billow_maxowe(PyObject* self, PyObject* arg) {
	return billow_diff(arg, Py_GT);
}

PyObject*
billow_minowe(PyObject* self, PyObject* arg) {
	return billow_diff(arg, Py_LT);
}

PyObject*
billow_rangowe(PyObject* self, PyObject* arg) {
	PyObject* min = NULL;
	PyObject* max = NULL;
	PyObject* first = NULL;
	PyObject* second = NULL;
	PyObject* it = NULL;
	int cmp;

	if ((it = PyObject_GetIter(arg)) == NULL)
		goto failed;
	for (;;) {
		if ((first = PyIter_Next(it)) == NULL) {
			if (PyErr_Occurred())
				goto failed;
			min = Py_Nil;
			Py_INCREF(min);
			max = Py_Nil;
			Py_INCREF(max);
			break;
		}
		if (first == Py_Nil) {
			Py_DECREF(first);
			continue;
		}
	continue_loop1:
		if ((second = PyIter_Next(it)) == NULL) {
			if (PyErr_Occurred())
				goto failed;
			min = first;
			max = first;
			Py_INCREF(max);
			first = NULL;
			break;
		}
		if (second == Py_Nil || (cmp = PyObject_Compare(first, second)) == 0) {
			Py_DECREF(second);
			goto continue_loop1;
		}
		if (cmp < 0) {
			if (PyErr_Occurred())
				goto failed;
			min = first;
			max = second;
		} else {
			min = second;
			max = first;
		}
		first = second = NULL;
	        for (;;) {
			if ((first = PyIter_Next(it)) == NULL) {
				if (PyErr_Occurred())
					goto failed;
				break;
			}
			if (first == Py_Nil) {
				Py_DECREF(first);
				continue;
			}
		continue_loop2:
			if ((second = PyIter_Next(it)) == NULL) {
				if (PyErr_Occurred())
					goto failed;
				if ((cmp = PyObject_RichCompareBool(first, min, Py_LT)) < 0)
					goto failed;
				if (cmp > 0) {
					Py_DECREF(min);
					min = first;
				} else {
					if ((cmp = PyObject_RichCompareBool(first, max, Py_GT)) < 0)
						goto failed;
					if (cmp > 0) {
						Py_DECREF(max);
						max = first;
					} else
						Py_DECREF(first);
				}
				first = NULL;
				break;
			}
			if (second == Py_Nil || (cmp = PyObject_Compare(first, second)) == 0) {
				Py_DECREF(second);
				goto continue_loop2;
			}
			if (cmp < 0) {
				if (PyErr_Occurred())
					goto failed;
				if ((cmp = PyObject_RichCompareBool(first, min, Py_LT)) < 0)
					goto failed;
				if (cmp) {
					Py_DECREF(min);
					min = first;
				} else
					Py_DECREF(first);
				first = NULL;
				if ((cmp = PyObject_RichCompareBool(second, max, Py_GT)) < 0)
					goto failed;
				if (cmp) {
					Py_DECREF(max);
					max = second;
				} else
					Py_DECREF(second);
				second = NULL;
			} else {
				if ((cmp = PyObject_RichCompareBool(second, min, Py_LT)) < 0)
					goto failed;
				if (cmp) {
					Py_DECREF(min);
					min = second;
				} else
					Py_DECREF(second);
				second = NULL;
				if ((cmp = PyObject_RichCompareBool(first, max, Py_GT)) < 0)
					goto failed;
				if (cmp) {
					Py_DECREF(max);
					max = first;
				} else
					Py_DECREF(first);
				first = NULL;
			}
		}
		break;
	}
	Py_DECREF(it);
	if ((it = PyTuple_New(2)) == NULL)
		goto failed;
	PyTuple_SET_ITEM(it, 0, min);
	PyTuple_SET_ITEM(it, 1, max);
	return it;
failed:
	Py_XDECREF(second);
	Py_XDECREF(first);
	Py_XDECREF(min);
	Py_XDECREF(max);
	Py_XDECREF(it);
	return NULL;
}
