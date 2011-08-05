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

#include "billow.h"
#include "billow_pair.h"
#include "pycompat.h"

#define BillowPair_NEW() (billow_pair*)BillowPair_Type.tp_alloc(&BillowPair_Type, 0)
#define BillowPair_SUBTYPE(__tp) (billow_pair*)(__tp)->tp_alloc(__tp, 0)

PyObject*
BillowPair_Pair(PyObject* k, PyObject* v) {
	register billow_pair* op;
	if ((op = BillowPair_NEW()) == NULL)
		return NULL;
	Py_INCREF(k);
	op->pair[0] = k;
	Py_INCREF(v);
	op->pair[1] = v;
	return (PyObject*)op;

}

static
void
billow_pair__dealloc(billow_pair* op) {
	_PyObject_GC_UNTRACK(op);
	Py_TRASHCAN_SAFE_BEGIN(op)
	Py_DECREF(op->pair[0]);
	Py_DECREF(op->pair[1]);
	Py_TYPE(op)->tp_free(op);
	Py_TRASHCAN_SAFE_END(op)
}

static
int
billow_pair__traverse(billow_pair* op, visitproc visit, void* arg) {
	if (visit(op->pair[0], arg) < 0)
		return -1;
	return visit(op->pair[1], arg);
}

static
PyObject*
billow_pair__new(PyTypeObject* tp, PyObject* args, PyObject* kwargs) {
	billow_pair* ret;
	PyObject* k;
	PyObject* v = NULL;
	if (kwargs != NULL) {
		PyErr_SetString(PyExc_TypeError, "pair() takes no keyword arguments");
		return NULL;
	}
	if (!PyArg_ParseTuple(args, "O|O:pair", &k, &v))
		return NULL;
	if (v != NULL) {
		Py_INCREF(k);
		Py_INCREF(v);
	} else {
		PyObject* iter;
		PyObject* next;
		if ((iter = PyObject_GetIter(k)) == NULL)
			return NULL;
		if ((k = PyIter_Next(iter)) == NULL) {
			Py_DECREF(iter);
			return NULL;
		}
		if ((v = PyIter_Next(iter)) == NULL) {
			Py_DECREF(iter);
			Py_DECREF(k);
			return NULL;
		}
		if ((next = PyIter_Next(iter)) != NULL) {
			PyErr_SetString(PyExc_ValueError, "length 2 of iterator is required");
			Py_DECREF(iter);
			Py_DECREF(next);
			goto failed;
		}
		if (PyErr_Occurred()) {
			Py_DECREF(iter);
			goto failed;
		}
		Py_DECREF(iter);
	}
	if ((ret = BillowPair_SUBTYPE(tp)) == NULL)
		goto failed;
	ret->pair[0] = k;
	ret->pair[1] = v;
	return (PyObject*)ret;
failed:
	Py_DECREF(k);
	Py_DECREF(v);
	return NULL;
}

static
PyObject*
billow_pair__repr(billow_pair* op) {
	PyObject* ret = NULL;
	PyObject* tuple = NULL;
	PyObject* sep = NULL;

	if ((tuple = PyTuple_New(6)) == NULL
	 || PyTuple_SET_ITEM(tuple, 0, PyString_FromString(Py_TYPE(op)->tp_name)) == NULL
	 || PyTuple_SET_ITEM(tuple, 1, PyString_FromString("(")) == NULL
	 || PyTuple_SET_ITEM(tuple, 2, PyObject_Repr(op->pair[0])) == NULL
	 || PyTuple_SET_ITEM(tuple, 3, PyString_FromString(", ")) == NULL
	 || PyTuple_SET_ITEM(tuple, 4, PyObject_Repr(op->pair[1])) == NULL
	 || PyTuple_SET_ITEM(tuple, 5, PyString_FromString(")")) == NULL
	 || (sep = PyString_FromString("")) == NULL) {
		Py_XDECREF(tuple);
		return NULL;
	}
	ret = _PyString_Join(sep, tuple);
	Py_XDECREF(sep);
	Py_XDECREF(tuple);
	return ret;
}

static
Py_ssize_t
billow_pair__length(billow_pair* op) {
	return 2;
}

static
PyObject*
billow_pair__getitem(billow_pair* op, Py_ssize_t i) {
	if ((i&~(Py_ssize_t)1) == 0) {
		register PyObject* ret;
		ret = op->pair[i];
		Py_INCREF(ret);
		return ret;
	} else {
		PyErr_Format(PyExc_IndexError, "%s index out of range", Py_TYPE(op)->tp_name);
		return NULL;
	}
}

static
PyObject*
billow_pair__getslice(billow_pair* op, register Py_ssize_t ilow, register Py_ssize_t ihigh) {
	billow_pair* ret;
	register PyObject* x;

	if ((ret = PyObject_GC_New(billow_pair, &BillowPairIter_Type)) == NULL)
		return NULL;
	if (ilow < 0)
		ilow = 0;
	if (ihigh > 2)
		ihigh = 2;
	if (((ihigh = ihigh - ilow - 1)&~(Py_ssize_t)1) != 0) {
		ret->pair[0] = NULL;
		ret->pair[1] = NULL;
	} else
	if (ihigh == 0) {
		x = op->pair[ilow];
		Py_INCREF(x);
		ret->pair[0] = x;
		ret->pair[1] = NULL;
	} else {
		x = op->pair[0];
		Py_INCREF(x);
		ret->pair[0] = x;
		x = op->pair[1];
		Py_INCREF(x);
		ret->pair[1] = x;
	}
	_PyObject_GC_TRACK(ret);
	return (PyObject*)ret;
}

static
PyObject*
billow_pair__subscript(billow_pair* op, PyObject* item) {
	if (PyIndex_Check(item)) {
		Py_ssize_t i = PyNumber_AsSsize_t(item, PyExc_IndexError);
		if (i < 0) {
			if (i == -1 && PyErr_Occurred())
				goto failed;
			i += 2;
		}
		if ((i&~(Py_ssize_t)1) == 0) {
			register PyObject* ret;
			ret = op->pair[i];
			Py_INCREF(ret);
			return ret;
		} else {
			PyErr_SetString(PyExc_IndexError, "pair index out of range");
			goto failed;
		}
	} else
	if (PySlice_Check(item)) {
		Py_ssize_t start;
		Py_ssize_t stop;
		Py_ssize_t step;
		billow_pair* ret;
		register PyObject* x;

		if ((x = ((PySliceObject*)item)->step) == Py_None)
			step = 1;
		else {
			if (!_PyEval_SliceIndex(x, &step))
				goto failed;
			if (step == 0) {
				PyErr_SetString(PyExc_ValueError, "slice step cannot be zero");
				goto failed;
			}
		}
		if ((x = ((PySliceObject*)item)->start) == Py_None)
			start = step < 0;
		else {
			if (!_PyEval_SliceIndex(x, &start))
				goto failed;
			if (start >= 0) {
				if (start >= 2)
					// if step is positive, they going
					// outer on the visible horizont.
					start = (step > 0) + 1;
			} else
			if ((start += 2) < 0)
				// as says bellow, but with negative sign.
				start = -(step < 0);
		}
		if ((x = ((PySliceObject*)item)->stop) == Py_None)
			stop = (step < 0) ? -1 : 2;
		else {
			if (!_PyEval_SliceIndex(x, &stop))
				goto failed;
			if (stop >= 0) {
				if (stop >= 2)
					stop = (step > 0) + 1;
			} else
			if ((stop += 2) < 0)
				stop = -(step < 0);
		}
		{
			register Py_ssize_t i = stop - start;
			// signs of the step and the vector of iteration
			// is complementary.
			if (i != 0 && (i^step) >= 0) {
				if ((i = start + step) == stop - step)
					stop = i;
				else
					stop = -1;
			} else
				start = -1;
		}
		if ((ret = PyObject_GC_New(billow_pair, &BillowPairIter_Type)) == NULL)
			goto failed;
		if ((start&~(Py_ssize_t)1) == 0) {
			x = op->pair[start];
			Py_INCREF(x);
			ret->pair[0] = x;
			if ((stop&~(Py_ssize_t)1) == 0) {
				x = op->pair[stop];
				Py_INCREF(x);
				ret->pair[1] = x;
			} else
				ret->pair[1] = NULL;
		} else {
			ret->pair[0] = NULL;
			ret->pair[1] = NULL;
		}
		_PyObject_GC_TRACK(ret);
		return (PyObject*)ret;
	} else
		PyErr_Format(PyExc_TypeError,
				"pair indices must be integers, not %.200s",
				Py_TYPE(item)->tp_name);
failed:
	return NULL;
}

static
int
billow_pair__contains(billow_pair* op, PyObject* el) {
	int cmp;

	if ((cmp = PyObject_RichCompareBool(el, op->pair[0], Py_EQ)) == 0)
		cmp = PyObject_RichCompareBool(el, op->pair[1], Py_EQ);
	return cmp;
}

static
PyObject*
billow_pair__richcompare(billow_pair* v, billow_pair* w, int op) {
	int eq;

	if (Py_TYPE(v) != &BillowPair_Type || Py_TYPE(w) != &BillowPair_Type) {
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	if ((eq = PyObject_RichCompareBool(v->pair[0], w->pair[0], Py_EQ)) < 0)
		return NULL;
	if (eq) {
		if ((eq = PyObject_RichCompareBool(v->pair[1], w->pair[1], Py_EQ)) < 0)
			return NULL;
		if (eq)
			switch (op) {
			case Py_LT:
			case Py_NE:
			case Py_GT:
				Py_INCREF(Py_False);
				return Py_False;
			case Py_LE:
			case Py_EQ:
			case Py_GE:
				Py_INCREF(Py_True);
				return Py_True;
			default: return NULL;
			}
		else
			switch (op) {
			case Py_EQ:
				Py_INCREF(Py_False);
				return Py_False;
			case Py_NE:
				Py_INCREF(Py_True);
				return Py_True;
			default:
				return PyObject_RichCompare(v->pair[1], w->pair[1], op);
			}

	} else
		switch (op) {
		case Py_EQ:
			Py_INCREF(Py_False);
			return Py_False;
		case Py_NE:
			Py_INCREF(Py_True);
			return Py_True;
		default:
			return PyObject_RichCompare(v->pair[0], w->pair[0], op);
		}
}

static
PyObject*
billow_pair_k__get(billow_pair* op, void* args) {
	PyObject* ret = op->pair[0];
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
billow_pair_v__get(billow_pair* op, void* args) {
	PyObject* ret = op->pair[1];
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
billow_pair_eqindex(billow_pair* op, PyObject* s) {
	int cmp;
	
	if ((cmp = PyObject_RichCompareBool(op->pair[0], s, Py_EQ)) > 0) {
		Py_INCREF(Py_False);
		return Py_False;
	}
	if (cmp < 0)
		return NULL;
	if ((cmp = PyObject_RichCompareBool(op->pair[1], s, Py_EQ)) > 0) {
		Py_INCREF(Py_True);
		return Py_True;
	}
	if (cmp == 0)
		PyErr_SetObject(PyExc_KeyError, s);
	return NULL;
}

static
PyObject*
billow_pair_count(billow_pair* op, PyObject* s) {
	long count = 0;
	int cmp;
	 
	if ((cmp = PyObject_RichCompareBool(op->pair[0], s, Py_EQ)) > 0)
		count++;
	else
	if (cmp < 0)
		return NULL;
	if ((cmp = PyObject_RichCompareBool(op->pair[1], s, Py_EQ)) > 0)
		count++;
	else
	if (cmp < 0)
		return NULL;
	return PyInt_FromLong(count);
}

static
PyObject*
billow_pair_with_k(billow_pair* op, PyObject* k) {
	if (op->pair[0] == k) {
		Py_INCREF(op);
		return (PyObject*)op;
	}
	return BillowPair_Pair(k, op->pair[1]);
}

static
PyObject*
billow_pair_with_v(billow_pair* op, PyObject* v) {
	if (op->pair[1] == v) {
		Py_INCREF(op);
		return (PyObject*)op;
	}
	return BillowPair_Pair(op->pair[0], v);
}

static
PyObject*
billow_pair_reverse(billow_pair* op) {
	return BillowPair_Pair(op->pair[1], op->pair[0]);
}

static
PyObject*
billow_pair__tuple(billow_pair* op) {
	PyObject* ret;

	if ((ret = PyTuple_New(2)) != NULL) {
		register PyObject* x;
		x = op->pair[0];
		Py_INCREF(x);
		PyTuple_SET_ITEM(ret, 0, x);
		x = op->pair[1];
		Py_INCREF(x);
		PyTuple_SET_ITEM(ret, 1, x);
	}
	return ret;
}

static
long
billow_pair__hash(billow_pair* op) {
	long mult = 1000003L;
	register long x;
	register long y;

	if ((y = PyObject_Hash(op->pair[0])) == -1)
		return -1;
	x = (0x345678L^y)*mult;
	mult += 82524L;
	if ((y = PyObject_Hash(op->pair[0])) == -1)
		return -1;
	x = (x^y)*mult;
	mult += 82524L;
	if ((x += 97531L) == -1)
		x = -2;
	return x;
}

static
PyObject*
billow_pair__iter(billow_pair* op) {
	billow_pair* it;

	if ((it = PyObject_GC_New(billow_pair, &BillowPairIter_Type)) != NULL) {
		register PyObject* x;
		x = op->pair[0];
		Py_INCREF(x);
		it->pair[0] = x;
		x = op->pair[1];
		Py_INCREF(x);
		it->pair[1] = x;
		_PyObject_GC_TRACK(it);
	}
	return (PyObject*)it;
}

PyDoc_STRVAR(doc_k,
"a key, specified the sequence index as 0");

PyDoc_STRVAR(doc_v,
"a value, specified the sequence index as 1");

PyDoc_STRVAR(doc_with_k,
"P.with_k(k) -> pair(k, P.v). Make a new pair with key k.");

PyDoc_STRVAR(doc_with_v,
"P.with_v(v) -> pair(P.k, v). Make a new pair with value v.");

PyDoc_STRVAR(doc_reverse,
"P.reverse() -> pair(P.v, P.k). Make a new pair in reverse order.");

PyDoc_STRVAR(doc_tuple,
"P.tuple() -> tuple(P). Convert pair to tuple.");

static PySequenceMethods billow_pair__sequence = {
	(lenfunc)billow_pair__length,		/* sq_length */
	0,					/* sq_concat */
	0,					/* sq_repeat */
	(ssizeargfunc)billow_pair__getitem,	/* sq_item */
	(ssizessizeargfunc)billow_pair__getslice,/* sq_slice */
	0,					/* sq_ass_item */
	0,					/* sq_ass_slice */
	(objobjproc)billow_pair__contains,	/* sq_contains */
};

static PyMappingMethods billow_pair__mapping = {
	(lenfunc)billow_pair__length,		/* mp_length */
	(binaryfunc)billow_pair__subscript,	/* mp_subscript */
	0
};

static PyMethodDef billow_pair__methods[] = {
{ "__getnewargs__", (PyCFunction)billow_pair__tuple, METH_NOARGS, NULL },
{ "eqindex", (PyCFunction)billow_pair_eqindex, METH_O, ""/* doc_eqindex */ },
{ "count", (PyCFunction)billow_pair_count, METH_O, ""/* doc_count */ },
{ "with_k", (PyCFunction)billow_pair_with_k, METH_O, doc_with_k },
{ "with_v", (PyCFunction)billow_pair_with_v, METH_O, doc_with_v },
{ "reverse", (PyCFunction)billow_pair_reverse, METH_NOARGS, doc_reverse },
{ "tuple", (PyCFunction)billow_pair__tuple, METH_NOARGS, doc_tuple },
{ NULL, NULL, 0, NULL }
};

static PyGetSetDef billow_pair__getset[] = {
{ "k", (getter)billow_pair_k__get, (setter)NULL, doc_k, NULL },
{ "v", (getter)billow_pair_v__get, (setter)NULL, doc_v, NULL },
{ NULL, NULL, NULL, NULL, NULL }
};

PyTypeObject
BillowPair_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"billow.pair",
	sizeof(billow_pair),
	0,
	(destructor)billow_pair__dealloc,	/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	(reprfunc)billow_pair__repr,		/* tp_repr */
	0,					/* tp_as_number */
	&billow_pair__sequence,			/* tp_as_sequence */
	&billow_pair__mapping,			/* tp_as_mapping */
	(hashfunc)billow_pair__hash,		/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,		/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"",//billow_pair_doc,			/* tp_doc */
	(traverseproc)billow_pair__traverse,	/* tp_traverse */
	0,					/* tp_clear */
	(richcmpfunc)billow_pair__richcompare,	/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	(getiterfunc)billow_pair__iter,		/* tp_iter */
	0,					/* tp_iternext */
	billow_pair__methods,			/* tp_methods */
	0,					/* tp_members */
	billow_pair__getset,			/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0,					/* tp_init */
	PyType_GenericAlloc,			/* tp_alloc */
	(newfunc)billow_pair__new,		/* tp_new */
	PyObject_GC_Del,			/* tp_free */
};

static
void
billow_pairiter__dealloc(billow_pair* op) {
	_PyObject_GC_UNTRACK(op);
	Py_TRASHCAN_SAFE_BEGIN(op)
	{
		register PyObject* x = op->pair[1];
		if (x != NULL) {
			Py_DECREF(x);
			x = op->pair[0];
			if (x != NULL) {
				Py_DECREF(x);
			}
		}
	}
	PyObject_GC_Del(op);
	Py_TRASHCAN_SAFE_END(op)
}

static
int
billow_pairiter__traverse(billow_pair* op, visitproc visit, void* arg) {
	if (op->pair[0] != NULL) {
		if (visit(op->pair[0], arg) < 0)
			return -1;
		if (op->pair[1] != NULL && visit(op->pair[1], arg) < 0)
			return -1;
	}
	return 0;
}

static
PyObject*
billow_pairiter__next(billow_pair* op) {
	PyObject* ret = op->pair[0];
	op->pair[0] = op->pair[1];
	op->pair[1] = NULL;
	return ret;
}

static
PyObject*
billow_pairiter__length(billow_pair* op) {
	return PyInt_FromLong((op->pair[0] == NULL) ? 0 : (op->pair[1] != NULL) + 1);
}

static PyMethodDef billow_pairiter__methods[] = {
	{ "__length_hint__", (PyCFunction)billow_pairiter__length, METH_NOARGS, NULL },
 	{ NULL, NULL }
};

PyTypeObject
BillowPairIter_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"billow.pair.iter",
	sizeof(billow_pair),
	0,
	(destructor)billow_pairiter__dealloc,	/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,		/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, /* tp_flags */
	0,					/* tp_doc */
	(traverseproc)billow_pairiter__traverse, /* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	PyObject_SelfIter,			/* tp_iter */
	(iternextfunc)billow_pairiter__next,	/* tp_iternext */
	billow_pairiter__methods,		/* tp_methods */
	0,
};

PyObject*
zippair(PyObject* self, PyObject* args) {
	PyObject* ret = NULL;
	billow_pair* itpair = NULL;
	PyObject* k = NULL;
	PyObject* v = NULL;
	PyObject* kit;
	PyObject* vit;
	Py_ssize_t i;
	Py_ssize_t len;

	if (!PyArg_UnpackTuple(args, "zippair", 2, 2, &kit, &vit))
		goto failed;
	if ((len = PyObject_LENGTH_HINT(kit)) >= 0) {
		if ((i = PyObject_LENGTH_HINT(vit)) < len)
			len = i;
	}
	if ((k = PyObject_GetIter(kit)) == NULL) {
		if (PyErr_ExceptionMatches(PyExc_TypeError))
			PyErr_SetString(PyExc_TypeError, "zippair first argument must support iteration");
		goto failed;
	}
	if ((v = PyObject_GetIter(vit)) == NULL) {
		if (PyErr_ExceptionMatches(PyExc_TypeError))
			PyErr_SetString(PyExc_TypeError, "zippair second argument must support iteration");
		goto failed;
	}
	if (len < 0)
		len = 10;
	if ((ret = PyList_New(len)) == NULL)
		goto failed;
	if ((itpair = BillowPair_NEW()) == NULL)
		goto failed;
	itpair->pair[0] = k;
	itpair->pair[1] = v;
	kit = k;
	vit = v;
	for (i = 0;; ++i) {
		if ((k = PyIter_Next(kit)) == NULL) {
			v = NULL;
			if (PyErr_Occurred())
				goto failed;
			break;
		}
		if ((v = PyIter_Next(vit)) == NULL) {
			if (PyErr_Occurred())
				goto failed;
			Py_CLEAR(k);
			break;
		} else {
			// in graph of execute path assign `v` to 'NULL' before
			// "goto failed" must be always true.
			register billow_pair* pair;
			if ((pair = BillowPair_NEW()) == NULL)
				goto failed;
			pair->pair[0] = k;
			pair->pair[1] = v;
			k = (PyObject*)pair;
		}
		// `v` referrent is borrowed by `k`-pair. Also take in mind
		// what `v` is point to nowere and make it in unsafed way for
		// more speed up.
		if (i < len)
			PyList_SET_ITEM(ret, i, k);
		else {
			if (PyList_Append(ret, k) < 0) {
				v = NULL;
				goto failed;
			}
			++len;
		}
	}
	// early free resource
	Py_CLEAR(itpair);
	if (i < len && PyList_SetSlice(ret, i, len, NULL) < 0)
		goto failed;
	return ret;
failed:
	Py_XDECREF(k);
	Py_XDECREF(v);
	Py_XDECREF(itpair);
	Py_XDECREF(ret);
	return NULL;
}
