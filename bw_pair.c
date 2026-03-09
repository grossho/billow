/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#include <Python.h>

#include "pycompat.h"
#include "billow.h"
#include "bw_pair.h"
#include "bw_debug.h"

PyObject*
bw_pair_New(PyObject* k, PyObject* v) {
	bw_pair_t* pair;

	if ((pair = (bw_pair_t*)bw_pair_Type.tp_alloc(&bw_pair_Type, 0)) == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(k);
	pair->pair_o[0] = k;
	Py_INCREF(v);
	pair->pair_o[1] = v;
	return (PyObject*)pair;
}

PyObject*
bw_pair_Borrowed(PyObject* k, PyObject* v) {
	bw_pair_t* pair;

	if ((pair = (bw_pair_t*)bw_pair_Type.tp_alloc(&bw_pair_Type, 0)) == NULL) {
		Py_DECREF(k);
		Py_DECREF(v);
		PyErr_NoMemory();
		return NULL;
	}
	pair->pair_o[0] = k;
	pair->pair_o[1] = v;
	return (PyObject*)pair;
}

static
void
bw_pair__tp_dealloc(bw_pair_t* pair) {
	Py_DECREF(pair->pair_o[1]);
	Py_DECREF(pair->pair_o[0]);
	Py_TYPE(pair)->tp_free(pair);
}

static
int
bw_pair__tp_traverse(bw_pair_t* pair, visitproc visit, void* arg) {
	if (visit(pair->pair_o[0], arg) < 0)
		return -1;
	return visit(pair->pair_o[1], arg);
}

static
PyObject*
bw_pair__tp_repr(bw_pair_t* pair) {
	PyObject* ret = NULL;
	PyObject* tuple = NULL;
	PyObject* s;
	PyObject* sep;

	Py_ssize_t i;

	if ((i = Py_ReprEnter((PyObject*)pair)) != 0)
		return i > 0 ? PyString_FromFormat("%s(...)", Py_TYPE(pair)->tp_name) : NULL;

	if ((tuple = PyTuple_New(6)) == NULL)
		goto failed;
	if ((s = PyString_FromString(Py_TYPE(pair)->tp_name)) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 0, s);
	if ((s = PyString_FromString("(")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 1, s);
	if ((s = PyObject_Repr(pair->pair_o[0])) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 2, s);
	if ((s = PyString_FromString(", ")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 3, s);
	if ((s = PyObject_Repr(pair->pair_o[1])) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 4, s);
	if ((s = PyString_FromString(")")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 5, s);

	if ((sep = PyString_FromString("")) == NULL)
		goto failed;
	ret = _PyString_Join(sep, tuple);
	Py_DECREF(sep);
failed:
	Py_XDECREF(tuple);
	Py_ReprLeave((PyObject *)pair);
	return ret;
}

static
PyObject*
bw_pair__tp_new(PyTypeObject* tp, PyObject* args, PyObject* kwargs) {
	bw_pair_t* ret;
	PyObject* k;
	PyObject* v = NULL;
	PyObject* iter = NULL;
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
		if ((iter = PyObject_GetIter(k)) == NULL)
			return NULL;
		if ((k = PyIter_Next(iter)) == NULL
		 || (v = PyIter_Next(iter)) == NULL) {
			if (!PyErr_Occurred())
				PyErr_SetString(PyExc_IndexError,
				 "must be at least 2 elements in iterator");
			goto failed;
		}
		Py_DECREF(iter);
	}
	if ((ret = (bw_pair_t*)(tp)->tp_alloc(tp, 0)) == NULL) {
		PyErr_NoMemory();
		goto failed;
	}
	ret->pair_o[0] = k;
	ret->pair_o[1] = v;
	return (PyObject*)ret;
failed:
	Py_XDECREF(k);
	Py_XDECREF(v);
	Py_XDECREF(iter);
	return NULL;
}

static
Py_ssize_t
bw_pair__mp_length(bw_pair_t* iter) {
	return (Py_ssize_t)2;
}

static
PyObject*
bw_pair__sq_getitem(bw_pair_t* pair, Py_ssize_t i) {
	if ((i&~(Py_ssize_t)1) == 0) {
		PyObject* ret = pair->pair_o[i];
		Py_INCREF(ret);
		return ret;
	} else {
		PyErr_Format(PyExc_IndexError, "getitem: %s index out of range", Py_TYPE(pair)->tp_name);
		return NULL;
	}
}

static
PyObject*
bw_pair__mp_subscript(bw_pair_t* pair, PyObject* item) {
	if (PyIndex_Check(item) > 0) {
		Py_ssize_t i = PyNumber_AsSsize_t(item, PyExc_IndexError);
		if (i == -1 && PyErr_Occurred())
			return NULL;
		if ((i&~(Py_ssize_t)1) == 0) {
			PyObject* ret;
			ret = pair->pair_o[i];
			Py_INCREF(ret);
			return ret;
		} else {
			PyErr_SetString(PyExc_IndexError,
				"subscript: pair index out of range");
			return NULL;
		}
	} else
	if (PySlice_Check(item) > 0) {
		Py_ssize_t start = 0;
		Py_ssize_t stop = 0;
		Py_ssize_t step = 0;
		bw_pair_t* ret;
		PyObject* p[2] = { NULL, NULL };
#if PY_VERSION_HEX < 0x03000000
#define PySlice_Unpack _PySlice_Unpack
#endif
		if (PySlice_Unpack(item, &start, &stop, &step) < 0)
			return NULL;
		if (start == stop)
			goto nulliter;
		if (step == 0) {
			if (start <= stop)
				step = 1;
			else
				step = -1;
		}
		if (start == stop)
			goto nulliter;
		if (step > 0) {
			if (start >= 2
			 || start >= stop)
				goto nulliter;
			if (start < 0) {
				if (step == 1)
					start = 0;
				else {
					start = step - (-start%step);
					if ((start&~(Py_ssize_t)1) != 0)
						goto nulliter;
				}
			}
			p[0] = pair->pair_o[start];
			if (step == 1
			 && (stop - start) >= 2)
				p[1] = pair->pair_o[start + 1];
		} else /* step < 0 */ {
			if (start < 0
			 || start < stop)
				goto nulliter;
			if (start >= 2) {
				if (step == -1)
					start = 1;
				else {
					start = start%-step;
					if ((start&~(Py_ssize_t)1) != 0)
						goto nulliter;
				}
			}
			p[0] = pair->pair_o[start];
			if (step == -1
			 && (start - stop) >= 2)
				p[1] = pair->pair_o[start - 1];
		}
	nulliter:
		if ((ret = (bw_pair_t*)bw_pair_iter_Type.tp_alloc(&bw_pair_iter_Type, 0)) == NULL) {
			PyErr_NoMemory();
			return NULL;
		}
		Py_XINCREF(p[0]);
		ret->pair_o[0] = p[0];
		Py_XINCREF(p[1]);
		ret->pair_o[1] = p[1];
		return (PyObject*)ret;
	} else {
		PyErr_Format(PyExc_TypeError,
			"pair indices must be integers, not %s",
			Py_TYPE(item)->tp_name);
		return NULL;
	}
}

static
int
bw_pair__sq_contains(bw_pair_t* pair, PyObject* el) {
	int cmp;

	if ((cmp = PyObject_RichCompareBool(el, pair->pair_o[0], Py_EQ)) == 0)
		cmp = PyObject_RichCompareBool(el, pair->pair_o[1], Py_EQ);
	return cmp;
}

static
PyObject*
bw_pair__tp_richcompare(bw_pair_t* pair, bw_pair_t* w, int op) {
	if (!PyObject_TypeCheck(w, &bw_pair_Type))
		return PyObject_RichCompare(pair->pair_o[0], (PyObject*)w, op);
	return PyObject_RichCompare(pair->pair_o[0], w->pair_o[0], op);
}

static
PyObject*
bw_pair__k__get(bw_pair_t* pair, void* args) {
	PyObject* ret = pair->pair_o[0];
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_pair__v__get(bw_pair_t* pair, void* args) {
	PyObject* ret = pair->pair_o[1];
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_pair__index(bw_pair_t* pair, PyObject* s) {
	PyObject* ret;
	int cmp;

	if ((cmp = PyObject_RichCompareBool(pair->pair_o[0], s, Py_EQ)) < 0)
		return NULL;
	if (cmp)
		ret = PyInt_FromLong(0);
	else {
		if ((cmp = PyObject_RichCompareBool(pair->pair_o[1], s, Py_EQ)) < 0)
			return NULL;
		if (cmp)
			ret = PyInt_FromLong(1);
		else {
			PyErr_SetObject(PyExc_KeyError, s);
			return NULL;
		}
	}
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_pair__indexbool(bw_pair_t* pair, PyObject* s) {
	PyObject* ret;
	int cmp;

	if ((cmp = PyObject_RichCompareBool(pair->pair_o[0], s, Py_EQ)) < 0)
		return NULL;
	if (cmp)
		ret = Py_False;
	else {
		if ((cmp = PyObject_RichCompareBool(pair->pair_o[1], s, Py_EQ)) < 0)
			return NULL;
		if (cmp)
			ret = Py_True;
		else {
			PyErr_SetObject(PyExc_KeyError, s);
			return NULL;
		}
	}
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_pair__count(bw_pair_t* pair, PyObject* s) {
	int cmp1;
	int cmp2;

	if ((cmp1 = PyObject_RichCompareBool(s, pair->pair_o[0], Py_EQ)) < 0
	 || (cmp2 = PyObject_RichCompareBool(s, pair->pair_o[1], Py_EQ)) < 0)
		return NULL;
	return PyInt_FromLong(cmp1 + cmp2);
}

static
PyObject*
bw_pair__with_k(bw_pair_t* pair, PyObject* k) {
	if (pair->pair_o[0] == k) {
		Py_INCREF(pair);
		return (PyObject*)pair;
	}
	return bw_pair_New(k, pair->pair_o[1]);
}

static
PyObject*
bw_pair__with_v(bw_pair_t* pair, PyObject* v) {
	if (pair->pair_o[1] == v) {
		Py_INCREF(pair);
		return (PyObject*)pair;
	}
	return bw_pair_New(pair->pair_o[0], v);
}

static
PyObject*
bw_pair__reversed(bw_pair_t* pair) {
	return bw_pair_New(pair->pair_o[1], pair->pair_o[0]);
}

static
PyObject*
bw_pair__tuple(bw_pair_t* pair) {
	PyObject* ret;

	if ((ret = PyTuple_New(2)) != NULL) {
		PyObject* x;
		x = pair->pair_o[0];
		Py_INCREF(x);
		PyTuple_SET_ITEM(ret, 0, x);
		x = pair->pair_o[1];
		Py_INCREF(x);
		PyTuple_SET_ITEM(ret, 1, x);
	}
	return ret;
}

static
long
bw_pair__tp_hash(bw_pair_t* pair) {
	return PyObject_Hash(pair->pair_o[0]);
}

static
PyObject*
bw_pair__tp_iter(bw_pair_t* pair) {
	bw_pair_t* iter;
	PyObject* x;

	if ((iter = (bw_pair_t*)bw_pair_iter_Type.tp_alloc(&bw_pair_iter_Type, 0)) == NULL) {
		PyErr_NoMemory();
		return NULL;
	}

	x = pair->pair_o[0];
	Py_INCREF(x);
	iter->pair_o[0] = x;

	x = pair->pair_o[1];
	Py_INCREF(x);
	iter->pair_o[1] = x;

	return (PyObject*)iter;
}

PyDoc_STRVAR(doc_k,
"a key, specified the sequence index as 0");

PyDoc_STRVAR(doc_v,
"a value, specified the sequence index as 1");

PyDoc_STRVAR(doc_with_k,
"P.with_k(k) -> pair(k, P.v). Make a new pair with key k.");

PyDoc_STRVAR(doc_with_v,
"P.with_v(v) -> pair(P.k, v). Make a new pair with value v.");

PyDoc_STRVAR(doc_reversed,
"P.__reversed__() -> pair(P.v, P.k). Make a new pair in reverse order.");

PyDoc_STRVAR(doc_tuple,
"P.tuple() -> tuple(P). Convert pair to tuple.");

static PyMethodDef bw_pair__tp_methods[] = {
{ "__getnewargs__", (PyCFunction)bw_pair__tuple, METH_NOARGS, NULL },
{ "index", (PyCFunction)bw_pair__index, METH_O, ""/* doc_indexbool */ },
{ "indexbool", (PyCFunction)bw_pair__indexbool, METH_O, ""/* doc_indexbool */ },
{ "count", (PyCFunction)bw_pair__count, METH_O, ""/* doc_count */ },
{ "with_k", (PyCFunction)bw_pair__with_k, METH_O, doc_with_k },
{ "with_v", (PyCFunction)bw_pair__with_v, METH_O, doc_with_v },
{ "fromlast", (PyCFunction)bw_pair__reversed, METH_NOARGS, doc_reversed },
{ "__reversed__", (PyCFunction)bw_pair__reversed, METH_NOARGS, doc_reversed },
{ "tuple", (PyCFunction)bw_pair__tuple, METH_NOARGS, doc_tuple },
{ NULL, NULL, 0, NULL }
};

static PyGetSetDef bw_pair__tp_getset[] = {
{ "k", (getter)bw_pair__k__get, (setter)NULL, doc_k, NULL },
{ "v", (getter)bw_pair__v__get, (setter)NULL, doc_v, NULL },
{ NULL, NULL, NULL, NULL, NULL }
};

static PySequenceMethods bw_pair__tp_as_sequence = {
	.sq_length = (lenfunc)bw_pair__mp_length,
	.sq_item = (ssizeargfunc)bw_pair__sq_getitem,
#if 0
#if PY_VERSION_HEX < 0x03000000
	.sq_slice = (ssizessizeargfunc)bw_pair__getslice,
#endif /* PY_VERSION_HEX < 0x03000000 */
#endif
	.sq_contains = (objobjproc)bw_pair__sq_contains,
};

static PyMappingMethods bw_pair__tp_as_mapping = {
	.mp_length = (lenfunc)bw_pair__mp_length,
	.mp_subscript = (binaryfunc)bw_pair__mp_subscript,
	.mp_ass_subscript = NULL,
};

PyTypeObject
bw_pair_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"billow.pair", sizeof(bw_pair_t), 0,
	.tp_dealloc = (destructor)bw_pair__tp_dealloc,
	.tp_repr = (reprfunc)bw_pair__tp_repr,
	.tp_as_sequence = &bw_pair__tp_as_sequence,
	.tp_as_mapping = &bw_pair__tp_as_mapping,
	.tp_hash = (hashfunc)bw_pair__tp_hash,
	//.tp_str = (strfunc)bw_pair__repr,
	.tp_getattro = PyObject_GenericGetAttr,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc = "",//bw_pair_doc,
	.tp_traverse = (traverseproc)bw_pair__tp_traverse,
	.tp_clear = NULL,			/* tp_clear */
	.tp_richcompare = (richcmpfunc)bw_pair__tp_richcompare,
	.tp_iter = (getiterfunc)bw_pair__tp_iter,
	.tp_iternext = 0,			/* tp_iternext */
	.tp_methods = bw_pair__tp_methods,
	.tp_getset = bw_pair__tp_getset,
	.tp_alloc = PyType_GenericAlloc,
	.tp_new = (newfunc)bw_pair__tp_new,
	.tp_free = PyObject_Del,
};

static
void
bw_pair_iter__tp_dealloc(bw_pair_t* pair) {
	Py_XDECREF(pair->pair_o[1]);
	Py_XDECREF(pair->pair_o[0]);
	Py_TYPE(pair)->tp_free(pair);
}

static
int
bw_pair_iter__tp_traverse(bw_pair_t* pair, visitproc visit, void* arg) {
	if (pair->pair_o[0] != NULL && visit(pair->pair_o[0], arg) < 0)
		return -1;
	if (pair->pair_o[1] != NULL && visit(pair->pair_o[1], arg) < 0)
		return -1;
	return 0;
}

static
PyObject*
bw_pair_iter__tp_iternext(bw_pair_t* pair) {
	PyObject* ret = pair->pair_o[0];
	pair->pair_o[0] = pair->pair_o[1];
	pair->pair_o[1] = NULL;
	return ret;
}

static
PyObject*
bw_pair_iter__length_hint(bw_pair_t* pair) {
	return PyInt_FromLong(
			(pair->pair_o[0] != NULL) +
			(pair->pair_o[1] != NULL)
	);
}

static PyMethodDef bw_pair_iter__methods[] = {
	{ "__length_hint__", (PyCFunction)bw_pair_iter__length_hint, METH_NOARGS, NULL },
	{ NULL, NULL }
};

PyTypeObject
bw_pair_iter_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"billow.pair.__iter__", sizeof(bw_pair_t), 0,
	.tp_dealloc = (destructor)bw_pair_iter__tp_dealloc,
	.tp_getattro = PyObject_GenericGetAttr,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "",
	.tp_traverse = (traverseproc)bw_pair_iter__tp_traverse,
	.tp_iter = PyObject_SelfIter,
	.tp_iternext = (iternextfunc)bw_pair_iter__tp_iternext,
	.tp_methods = bw_pair_iter__methods,
};

// function for billow.c
PyObject*
bw_zippair(PyObject* self, PyObject* args) {
	PyObject* ret = NULL;
	PyObject* k = NULL;
	PyObject* v = NULL;
	PyObject* kit;
	PyObject* vit;

	if (!PyArg_UnpackTuple(args, "zippair", 2, 2, &kit, &vit))
		goto failed;
	if ((k = PyObject_GetIter(kit)) == NULL) {
		if (PyErr_ExceptionMatches(PyExc_TypeError))
			PyErr_SetString(PyExc_TypeError,
				"zippair first argument must support iteration");
		goto failed;
	}
	if ((v = PyObject_GetIter(vit)) == NULL) {
		if (PyErr_ExceptionMatches(PyExc_TypeError))
			PyErr_SetString(PyExc_TypeError,
				"zippair second argument must support iteration");
		goto failed;
	}
	if ((ret = PyList_New(0)) == NULL)
		goto failed;
	kit = k;
	vit = v;
	k = v = NULL;
	for (;;) {
		BW_YIELD();
		if ((k = PyIter_Next(kit)) == NULL) {
			if (PyErr_Occurred()) {
				v = NULL;
				goto failed_loop;
			}
			break;
		}
		if ((v = PyIter_Next(vit)) != NULL) {
			PyObject* pair;
			if ((pair = bw_pair_Borrowed(k, v)) == NULL)
				goto failed_loop;
			if (PyList_Append(ret, pair) < 0) {
				k = v = NULL;
				Py_DECREF(pair);
				goto failed_loop;
			}
			Py_DECREF(pair);
		} else {
			if (PyErr_Occurred())
				goto failed_loop;
			Py_CLEAR(k);
			break;
		}
	}
	Py_DECREF(vit);
	Py_DECREF(kit);
	return ret;
failed_loop:
	Py_DECREF(vit);
	Py_DECREF(kit);
failed:
	Py_XDECREF(k);
	Py_XDECREF(v);
	Py_XDECREF(ret);
	return NULL;
}

