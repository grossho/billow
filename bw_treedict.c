/*
 * (c) Vladimir Yu. Stepanov 2006-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#include <string.h>

#include <Python.h>

#include "billow.h"
#include "px_types.h"
#include "px_genlink.h"
#include "pxgtc_balance.h"
#include "pxgtc_gentree.h"
#include "bw_treedict.h"

#define bw_treedict__wrlock_SET(x, r)				\
	do {							\
		if ((x)->td_lockcount != 0) {			\
			if (td->td_lockcount > 0)		\
				PyErr_SetNone(BwExc_RDLockError);\
			else					\
				PyErr_SetNone(BwExc_WRLockError);\
			return r;				\
		}						\
		(x)->td_lockcount -= 1;			\
	} while (0)

#define bw_treedict__wrlock_UNSET(x)				\
	do {							\
		(x)->td_lockcount += 1;			\
	} while (0)

#define bw_treedict__rdlock_SET(x, r)				\
	do {							\
		if ((x)->td_lockcount < 0) {			\
			PyErr_SetNone(BwExc_WRLockError);	\
			return r;				\
		}						\
		(x)->td_lockcount += 1;			\
	} while (0)

#define bw_treedict__rdlock_UNSET(x)				\
	do {							\
		(x)->td_lockcount -= 1;			\
	} while(0)

static
_u3
bw_treedict__getstate_avl(const pxgtc_algo_t* algo, px_genlink_t* n) {
	return (_u3)(((bw_treedict_node_t*)n)->n_state_avl);
}

static
void
bw_treedict__setstate_avl(const pxgtc_algo_t* algo, px_genlink_t* n, _u3 state) {
	((bw_treedict_node_t*)n)->n_state_avl = state;
}

static
_u3
bw_treedict__getstate_rb(const pxgtc_algo_t* algo, px_genlink_t* n) {
	return (_u3)(((bw_treedict_node_t*)n)->n_state_rb);
}

static
void
bw_treedict__setstate_rb(const pxgtc_algo_t* algo, px_genlink_t* n, _u3 state) {
	((bw_treedict_node_t*)n)->n_state_rb = state;
}

pxgtc_algo_t bw_treedict__algo_list[] = {
{
	.algo_getstate = bw_treedict__getstate_avl,
	.algo_setstate = bw_treedict__setstate_avl,
	.algo_getw5 = NULL,
	.algo_setw5 = NULL,
	.algo_balance_insert = pxgtc_balance_avl_insert,
	.algo_balance_remove = pxgtc_balance_avl_remove,
}, {
	.algo_getstate = bw_treedict__getstate_rb,
	.algo_setstate = bw_treedict__setstate_rb,
	.algo_getw5 = NULL,
	.algo_setw5 = NULL,
	.algo_balance_insert = pxgtc_balance_rb_insert,
	.algo_balance_remove = pxgtc_balance_rb_remove,
}
};

static
void
bw_treedict__default(bw_treedict_t* td) {
	td->td_lockcount = 0;
	td->td_length = 0;
	td->td_root = NULL;
	td->td_compare = NULL;
	td->td_algo = NULL;
}

static
_sA
bw_treedict__tp_traverse(bw_treedict_t* td, visitproc visit, void* arg) {
	px_genlink_t*** k;
	px_genlink_t** path[PxGTC_MAXPATH];

	if (td->td_compare != NULL && visit(td->td_compare, arg) < 0)
		return -1;
	k = pxgtc_pathmin(path, &td->td_root);
	while ((k = pxgtc_znext(path, k)) != NULL) {
		if (visit(((bw_treedict_node_t*)**k)->n_key, arg) < 0)
			return -1;
		if (visit(((bw_treedict_node_t*)**k)->n_value, arg) < 0)
			return -1;
	}
	return 0;
}

static
_sA
bw_treedict__tp_clear(bw_treedict_t* td) {
	px_genlink_t*** k;
	px_genlink_t*** path = td->td_path;

	td->td_lockcount = -1; // bw_treedict__wrlock_SET absolutely
	td->td_length = 0;
	k = pxgtc_pathmin(path, &td->td_root);
	while (((k = pxgtc_znext(path, k))) != NULL) {
		px_genlink_t* p = **k;
		Py_DECREF(((bw_treedict_node_t*)p)->n_key);
		Py_DECREF(((bw_treedict_node_t*)p)->n_value);
		PyMem_FREE(p);
	}
	td->td_root = NULL;
	Py_CLEAR(td->td_compare);
	bw_treedict__default(td);
	return 0;
}

static
void
bw_treedict__tp_dealloc(bw_treedict_t* td) {
	PyObject_GC_UnTrack((PyObject*)td);
#if PY_VERSION_HEX < 0x03080000
	Py_TRASHCAN_SAFE_BEGIN(td)
#else /* PY_VERSION_HEX < 0x03080000 */
	Py_TRASHCAN_BEGIN(td, bw_treedict__tp_dealloc)
#endif /* PY_VERSION_HEX < 0x03080000 */
	bw_treedict__tp_clear(td);
	PyObject_GC_Del(td);
#if PY_VERSION_HEX < 0x03080000
	Py_TRASHCAN_SAFE_END(td)
#else /* PY_VERSION_HEX < 0x03080000 */
	Py_TRASHCAN_END
#endif /* PY_VERSION_HEX < 0x03080000 */
}

bw_treedict_node_t*
bw_treedict__getbykey(bw_treedict_t* td, PyObject* key) {
	px_genlink_t* root = td->td_root;
	PyObject* compare;
	_s_long cmp;

	if ((compare = td->td_compare) != NULL) {
		PyObject* ret;

		while (root != NULL) {
			if ((ret = PyObject_CallFunction(compare, "OO",
					key, ((bw_treedict_node_t*)root)->n_key)) == NULL)
				return NULL;
			if (!PyInt_Check(ret)) {
				Py_DECREF(ret);
				PyErr_SetString(PyExc_TypeError,
					"compare function should return an int()");
				return NULL;
			}
			cmp = PyInt_AsLong(ret);
			Py_DECREF(ret);
			if (cmp == 0)
				return ((bw_treedict_node_t*)root);
			root = root->link[cmp > 0];
		}
	} else {
		while (root != NULL) {
			if ((cmp = PyObject_Compare(key,
					((bw_treedict_node_t*)root)->n_key)) < 0) {
				if (PyErr_Occurred())
					return NULL;
			}
			if (cmp == 0)
				return ((bw_treedict_node_t*)root);
			root = root->link[cmp > 0];
		}
	}
	return NULL;
}

px_genlink_t***
bw_treedict__getbypath(bw_treedict_t* td, px_genlink_t*** path, PyObject* key) {
	px_genlink_t** pp = &td->td_root;
	PyObject* compare;
	_s_long cmp;

	if ((compare = td->td_compare) != NULL) {
		PyObject* ret;
		while (*pp != NULL) {
			if ((ret = PyObject_CallFunction(compare, "OO", key, ((bw_treedict_node_t*)*pp)->n_key)) == NULL)
				return NULL;
			if (!PyInt_Check(ret)) {
				Py_DECREF(ret);
				PyErr_SetString(PyExc_TypeError,
					"compare function should return an int()");
				return NULL;
			}
			cmp = PyInt_AsLong(ret);
			Py_DECREF(ret);
			if (cmp == 0)
				break;
			*path++ = pp;
			pp = &(*pp)->link[cmp > 0];
		}
	} else {
		while (*pp != NULL) {
			if ((cmp = PyObject_Compare(key,
					((bw_treedict_node_t*)*pp)->n_key)) < 0) {
				if (PyErr_Occurred())
					return NULL;
			}
			if (cmp == 0)
				break;
			*path++ = pp;
			pp = &(*pp)->link[cmp > 0];
		}
	}
	*path = pp;
	return path;
}

static PyObject* bw_treedict__update(bw_treedict_t* td, PyObject* arg);

static
PyObject*
bw_treedict__tp_new(PyTypeObject* tp, PyObject* args, PyObject* kwargs) {
	static char* kwlist[] = {
		"seq",
		"compare",
		"algono",
		NULL,
	};
	bw_treedict_t* td;
	PyObject* seq = NULL;
	PyObject* compare = NULL;
	_sA algono = 0;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOi:__new__", kwlist,
				&seq, &compare, &algono))
		return NULL;
	if (compare == Py_None)
		compare = NULL;
	if (algono < 0)
		algono = 0;
	if (algono > 1)
		algono = 1;
	if (compare != NULL) {
		if (compare == Py_None)
			compare = NULL;
		else
		if (!PyCallable_Check(compare)) {
			PyErr_SetString(PyExc_TypeError,
					"compare require an callable");
			return NULL;
		}
	}
	if ((td = (bw_treedict_t*)tp->tp_alloc(tp, 0)) == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	bw_treedict__default(td);
	if (compare != NULL) {
		Py_INCREF(compare);
		td->td_compare = compare;
	}
	td->td_algo = &bw_treedict__algo_list[ algono ];
	if (seq != NULL) {
		PyObject* ret;
		if ((ret = bw_treedict__update(td, seq)) == NULL) {
			Py_DECREF(td);
			return NULL;
		}
		Py_DECREF(ret);
	}
	return (PyObject*)td;
}

static
PyObject*
bw_treedict__fromkeys(PyTypeObject* tp, PyObject* args, PyObject* kwargs) {
	static char* kwlist[] = {
		"seq",
		"default",
		"compare",
		"algono",
		NULL,
	};
	bw_treedict_t* td;
	PyObject* seq = NULL;
	PyObject* def = Py_None;
	PyObject* compare = NULL;
	PyObject* iter = NULL;
	PyObject* next = NULL;
	bw_treedict_node_t* n;
	px_genlink_t*** k;
	pxgtc_algo_t* algo;
	_sA algono = 0;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|OOi:__new__", kwlist,
				&seq, &def, &compare, &algono))
		return NULL;
	if (compare == Py_None)
		compare = NULL;
	if (algono < 0)
		algono = 0;
	if (algono > 1)
		algono = 1;
	if (compare != NULL) {
		if (compare == Py_None)
			compare = NULL;
		else
		if (!PyCallable_Check(compare)) {
			PyErr_SetString(PyExc_TypeError,
					"compare require an callable");
			return NULL;
		}
	}
	if ((td = (bw_treedict_t*)tp->tp_alloc(tp, 0)) == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	bw_treedict__default(td);
	if (compare != NULL) {
		Py_INCREF(compare);
		td->td_compare = compare;
	}
	td->td_algo = algo = &bw_treedict__algo_list[ algono ];

	if ((iter = PyObject_GetIter(seq)) == NULL)
		goto failed;
	for (;;) {
		if ((next = PyIter_Next(iter)) == NULL) {
			if (PyErr_Occurred())
				goto failed;
			break;
		}
		if ((k = bw_treedict__getbypath(td, td->td_path, next)) == NULL)
			goto failed;
		if ((n = (bw_treedict_node_t*)**k) == NULL) {
			if ((n = PyMem_MALLOC(sizeof(*n))) == NULL)
				goto failed;
			//n->n_link.link[0] = n->n_link.link[1] = NULL;
			//algo->algo_setstate(algo, n, 0);
			// borrowed refcount
			n->n_key = next;
			next = NULL;
			Py_INCREF(def);
			n->n_value = def;
			**k = &n->n_link;
			td->td_length++;
			algo->algo_balance_insert(algo, td->td_path, k);
		} else {
			Py_DECREF(next);
			next = NULL;
		}
	}
	Py_DECREF(iter);
	return (PyObject*)td;
failed:
	Py_XDECREF(iter);
	Py_XDECREF(next);
	Py_XDECREF(td);
	return NULL;
}

#if PY_VERSION_HEX < 0x03030000
static
PyObject*
bw_treedict__tp_repr(bw_treedict_t* td) {
	PyObject* repr = NULL;
	PyObject* list = NULL;
	PyObject* tuple = NULL;
	PyObject* sep = NULL;
	PyObject* item = NULL;
	PyObject* s;
	px_genlink_t*** k;
	bw_treedict_node_t* n;
	_sA i;
	
	if ((i = Py_ReprEnter((PyObject*)td)) != 0)
		return i > 0 ? PyString_FromFormat("%s(...)", Py_TYPE(td)->tp_name) : NULL;
	i = 0;
	bw_treedict__rdlock_SET(td, NULL);
	if (td->td_compare != NULL) {
		if ((tuple = PyTuple_New(8)) == NULL)
			goto failed;
	} else {
		if ((tuple = PyTuple_New(6)) == NULL)
			goto failed;
	}
	if ((list = PyList_New(0)) == NULL)
		goto failed;

	k = pxgtc_pathmin(td->td_path, &td->td_root);
	for (;;) {
		BW_YIELD();
		if ((k = pxgtc_wnext(td->td_path, k)) == NULL)
			break;
		n = (bw_treedict_node_t*)**k;
		if ((item = PyTuple_New(2)) == NULL)
			goto failed;
		Py_INCREF(n->n_key);
		PyTuple_SET_ITEM(item, 0, n->n_key);
		Py_INCREF(n->n_value);
		PyTuple_SET_ITEM(item, 1, n->n_value);
		if (PyList_Append(list, item) < 0)
			goto failed;
		Py_CLEAR(item);
	}
	if ((s = PyString_FromString(Py_TYPE(td)->tp_name)) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, i++, s);
	if ((s = PyString_FromString("(")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, i++, s);
	if ((s = PyObject_Repr(list)) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, i++, s);

	if (td->td_compare != NULL) {
		if ((s = PyString_FromString(", compare=")) == NULL)
			goto failed;
		PyTuple_SET_ITEM(tuple, i++, s);
		if ((s = PyObject_Repr(td->td_compare)) == NULL)
			goto failed;
		PyTuple_SET_ITEM(tuple, i++, s);
	}

	if ((s = PyString_FromString(", algono=")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, i++, s);
	if ((s = PyString_FromFormat("%d", td->td_algo - bw_treedict__algo_list)) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, i++, s);
	if ((s = PyString_FromString(")")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, i++, s);

	if ((sep = PyString_FromString("")) == NULL)
		goto failed;
	repr = _PyString_Join(sep, tuple);
failed:
	bw_treedict__rdlock_UNSET(td);
	Py_ReprLeave((PyObject*)td);
	Py_XDECREF(item);
	Py_XDECREF(sep);
	Py_XDECREF(tuple);
	Py_XDECREF(list);
	return repr;
}
#else /* PY_VERSION_HEX < 0x03030000 */
static
PyObject*
bw_treedict__tp_repr(bw_treedict_t* td) {
#if PY_VERSION_HEX < 0x030e0000
	PyUnicodeWriter writerobj;
#endif /* PY_VERSION_HEX < 0x030e0000 */
	PyUnicodeWriter* writer;
	PyObject* s = NULL;
	px_genlink_t*** k;
	_uA i;

	bw_treedict__rdlock_SET(td, NULL);
	if ((i = Py_ReprEnter((PyObject*)td)) != 0) {
		bw_treedict__rdlock_UNSET(td);
		return i > 0 ? PyString_FromFormat("%s(...)", Py_TYPE(td)->tp_name) : NULL;
	}
#if PY_VERSION_HEX < 0x030e0000
	writer = &writerobj;
	_PyUnicodeWriter_Init(writer);
#else
	writer = PyUnicodeWriter_Create(0);
#endif
	if (PyUnicodeWriter_WriteASCII(writer, Py_TYPE(td)->tp_name, strlen(Py_TYPE(td)->tp_name)) < 0)
		goto failed;
	if (PyUnicodeWriter_WriteChar(writer, '(') < 0)
		goto failed;
	if (PyUnicodeWriter_WriteChar(writer, '[') < 0)
		goto failed;
	i = 0;
	k = pxgtc_pathmin(td->td_path, &td->td_root);
	for (i = 0; (k = pxgtc_wnext(td->td_path, k)) != NULL; i++) {
		bw_treedict_node_t* n = ((bw_treedict_node_t*)**k);
		if (i != 0)
			if (PyUnicodeWriter_WriteASCII(writer, ", ", 2) < 0)
				goto failed;
		if (PyUnicodeWriter_WriteChar(writer, '(') < 0)
			goto failed;
		if ((s = PyObject_Repr(n->n_key)) == NULL)
			goto failed;
		if (PyUnicodeWriter_WriteStr(writer, s) < 0)
			goto failed;
		Py_CLEAR(s);
		if (PyUnicodeWriter_WriteASCII(writer, ", ", 2) < 0)
			goto failed;
		if ((s = PyObject_Repr(n->n_value)) == NULL)
			goto failed;
		if (PyUnicodeWriter_WriteStr(writer, s) < 0) 
			goto failed;
		Py_CLEAR(s);
		if (PyUnicodeWriter_WriteChar(writer, ')') < 0)
			goto failed;
	}
	if (PyUnicodeWriter_WriteChar(writer, ']') < 0)
		goto failed;
	if (td->td_compare != NULL) {
		if (PyUnicodeWriter_WriteASCII(writer, ", ", 2) < 0)
			goto failed;
		if (PyUnicodeWriter_WriteASCII(writer, "compare=", 8) < 0)
			goto failed;
		if ((s = PyObject_Repr(td->td_compare)) == NULL)
			goto failed;
		if (PyUnicodeWriter_WriteStr(writer, s) < 0)
			goto failed;
		Py_CLEAR(s);
	}
	if (PyUnicodeWriter_WriteASCII(writer, ", algono=", 9) < 0)
		goto failed;
	if (PyUnicodeWriter_WriteChar(writer,
			"01"[td->td_algo - bw_treedict__algo_list]) < 0)
		goto failed;

	if (PyUnicodeWriter_WriteChar(writer, ')') < 0)
		goto failed;
	Py_ReprLeave((PyObject *)td);
	bw_treedict__rdlock_UNSET(td);
	return PyUnicodeWriter_Finish(writer);

failed:
	Py_XDECREF(s);
	PyUnicodeWriter_Discard(writer);
	Py_ReprLeave((PyObject *)td);
	bw_treedict__rdlock_UNSET(td);
	return NULL;
}
#endif /* PY_VERSION_HEX < 0x03030000 */

static
PyObject*
bw_treedict__get(bw_treedict_t* td, PyObject* args) {
	PyObject* ret = Py_None;
	PyObject* key;
	bw_treedict_node_t* n;

	if (!PyArg_UnpackTuple(args, "get", 1, 2, &key, &ret))
		return NULL;
	bw_treedict__rdlock_SET(td, NULL);
	if ((n = bw_treedict__getbykey(td, key)) != NULL) {
		ret = n->n_value;
		Py_INCREF(ret);
	} else
	if (PyErr_Occurred())
		ret = NULL;
	else
		Py_INCREF(ret);
	bw_treedict__rdlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__setdefault(bw_treedict_t* td, PyObject* args) {
	PyObject* ret = NULL;
	pxgtc_algo_t* algo = td->td_algo;
	PyObject* key;
	PyObject* value = Py_None;
	px_genlink_t*** k;
	bw_treedict_node_t* n;

	if (!PyArg_UnpackTuple(args, "setdefault", 1, 2, &key, &value))
		return NULL;
	bw_treedict__wrlock_SET(td, NULL);
	if ((k = bw_treedict__getbypath(td, td->td_path, key)) == NULL)
		goto failed;
	if ((n = (bw_treedict_node_t*)**k) == NULL) {
		if ((n = PyMem_MALLOC(sizeof(*n))) == NULL) {
			PyErr_NoMemory();
			goto failed;
		}
		//n->n_link.link[0] = n->n_link.link[1] = NULL;
		//algo->algo_setstate(algo, n, 0);
		Py_INCREF(key);
		n->n_key = key;
		Py_INCREF(value);
		n->n_value = value;
		**k = &n->n_link;
		td->td_length++;
		algo->algo_balance_insert(algo, td->td_path, k);
		ret = value;
	} else
		ret = n->n_value;
	Py_INCREF(ret);
failed:
	bw_treedict__wrlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__replace(bw_treedict_t* td, PyObject* args) {
	PyObject* ret = NULL;
	PyObject* key;
	PyObject* value;
	bw_treedict_node_t* n;

	if (!PyArg_UnpackTuple(args, "replace", 2, 2, &key, &value))
		return NULL;
	bw_treedict__wrlock_SET(td, NULL);
	if ((n = bw_treedict__getbykey(td, key)) == NULL) {
		PyErr_SetString(PyExc_KeyError, "key not found");
		goto failed;
	}
	Py_INCREF(value);
	Py_DECREF(n->n_value);
	n->n_value = value;
	ret = Py_None;
	Py_INCREF(ret);
failed:
	bw_treedict__wrlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__pop(bw_treedict_t* td, PyObject* args) {
	PyObject* ret = Py_None;
	pxgtc_algo_t* algo = td->td_algo;
	PyObject* key;
	px_genlink_t*** k;
	bw_treedict_node_t* n;

	if (!PyArg_UnpackTuple(args, "pop", 1, 2, &key, &ret))
		return NULL;
	bw_treedict__wrlock_SET(td, NULL);
	if ((k = bw_treedict__getbypath(td, td->td_path, key)) == NULL) {
		ret = NULL;
		goto failed;
	}
	if ((n = (bw_treedict_node_t*)**k) == NULL) {
		Py_INCREF(ret);
		goto failed;
	}
	Py_DECREF(n->n_key);
	// borrowed refcount
	ret = n->n_value;
	td->td_length--;
	algo->algo_balance_remove(algo, td->td_path, k);
	PyMem_FREE(n);
failed:
	bw_treedict__wrlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__popmin(bw_treedict_t* td) {
	PyObject* ret = NULL;
	pxgtc_algo_t* algo = td->td_algo;
	px_genlink_t*** k;
	bw_treedict_node_t* n;

	bw_treedict__wrlock_SET(td, NULL);
	if (td->td_length == 0) {
		PyErr_SetNone(PyExc_KeyError);
		goto failed;
	}
	k = pxgtc_pathmin(td->td_path, &td->td_root) - 1;
	n = ((bw_treedict_node_t*)**k);
	Py_DECREF(n->n_key);
	// borrowed refcount
	ret = n->n_value;
	td->td_length--;
	algo->algo_balance_remove(algo, td->td_path, k);
	PyMem_FREE(n);
failed:
	bw_treedict__wrlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__popmax(bw_treedict_t* td) {
	PyObject* ret = NULL;
	pxgtc_algo_t* algo = td->td_algo;
	px_genlink_t*** k;
	bw_treedict_node_t* n;

	bw_treedict__wrlock_SET(td, NULL);
	if (td->td_length == 0) {
		PyErr_SetNone(PyExc_KeyError);
		goto failed;
	}
	k = pxgtc_pathmax(td->td_path, &td->td_root) - 1;
	n = ((bw_treedict_node_t*)**k);
	Py_DECREF(n->n_key);
	// borrowed refcount
	ret = n->n_value;
	td->td_length--;
	algo->algo_balance_remove(algo, td->td_path, k);
	PyMem_FREE(n);
failed:
	bw_treedict__wrlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__poppairmin(bw_treedict_t* td) {
	pxgtc_algo_t* algo = td->td_algo;
	PyObject* ret = NULL;
	px_genlink_t*** k;
	bw_treedict_node_t* n;

	bw_treedict__wrlock_SET(td, NULL);
	if (td->td_length == 0) {
		PyErr_SetNone(PyExc_KeyError);
		goto failed;
	}
	k = pxgtc_pathmin(td->td_path, &td->td_root) - 1;
	n = ((bw_treedict_node_t*)**k);
	// borrowed refcount's
	if ((ret = bw_pair_Borrowed(n->n_key, n->n_value)) == NULL)
		goto failed;
	td->td_length--;
	algo->algo_balance_remove(algo, td->td_path, k);
	PyMem_FREE(n);
failed:
	bw_treedict__wrlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__poppairmax(bw_treedict_t* td) {
	pxgtc_algo_t* algo = td->td_algo;
	PyObject* ret = NULL;
	px_genlink_t*** k;
	bw_treedict_node_t* n;

	bw_treedict__wrlock_SET(td, NULL);
	if (td->td_length == 0) {
		PyErr_SetNone(PyExc_KeyError);
		goto failed;
	}
	k = pxgtc_pathmax(td->td_path, &td->td_root) - 1;
	n = ((bw_treedict_node_t*)**k);
	// borrowed refcount's
	if ((ret = bw_pair_Borrowed(n->n_key, n->n_value)) == NULL)
		goto failed;
	td->td_length--;
	algo->algo_balance_remove(algo, td->td_path, k);
	PyMem_FREE(n);
failed:
	bw_treedict__wrlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__pushmin(bw_treedict_t* td, PyObject* args) {
	PyObject* ret = NULL;
	PyObject* key;
	PyObject* value;
	pxgtc_algo_t* algo = td->td_algo;
	px_genlink_t*** k;
	bw_treedict_node_t* n;

	if (!PyArg_UnpackTuple(args, "pushmin", 2, 2, &key, &value))
		return NULL;
	bw_treedict__wrlock_SET(td, NULL);
	k = pxgtc_pathmin(td->td_path, &td->td_root);
	if ((n = PyMem_MALLOC(sizeof(*n))) == NULL) {
		PyErr_NoMemory();
		goto failed;
	}
	//n->n_link.link[0] = n->n_link.link[1] = NULL;
	//algo->algo_setstate(algo, n, 0);
	Py_INCREF(key);
	n->n_key = key;
	Py_INCREF(value);
	n->n_value = value;
	**k = &n->n_link;
	td->td_length++;
	algo->algo_balance_insert(algo, td->td_path, k);
	Py_INCREF(key);
	ret = key;
failed:
	bw_treedict__wrlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__pushmax(bw_treedict_t* td, PyObject* args) {
	PyObject* ret = NULL;
	PyObject* key;
	PyObject* value;
	pxgtc_algo_t* algo = td->td_algo;
	px_genlink_t*** k;
	bw_treedict_node_t* n;

	if (!PyArg_UnpackTuple(args, "pushmax", 2, 2, &key, &value))
		return NULL;
	bw_treedict__wrlock_SET(td, NULL);
	k = pxgtc_pathmax(td->td_path, &td->td_root);
	if ((n = PyMem_MALLOC(sizeof(*n))) == NULL) {
		PyErr_NoMemory();
		goto failed;
	}
	//n->n_link.link[0] = n->n_link.link[1] = NULL;
	//algo->algo_setstate(algo, n, 0);
	Py_INCREF(key);
	n->n_key = key;
	Py_INCREF(value);
	n->n_value = value;
	**k = &n->n_link;
	td->td_length++;
	algo->algo_balance_insert(algo, td->td_path, k);
	Py_INCREF(key);
	ret = key;
failed:
	bw_treedict__wrlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__keys_inc(bw_treedict_t* td) {
	PyObject* ret;

	bw_treedict__rdlock_SET(td, NULL);
	if ((ret = PyList_New(td->td_length)) != NULL) {
		_sA i;
		px_genlink_t*** k;

		k = pxgtc_pathmin(td->td_path, &td->td_root);
		for (i = 0; (k = pxgtc_wnext(td->td_path, k)) != NULL; i++) {
			PyObject* key = ((bw_treedict_node_t*)**k)->n_key;
			Py_INCREF(key);
			PyList_SET_ITEM(ret, i, key);
		}
	}
	bw_treedict__rdlock_UNSET(td);
	return ret;
}
static
PyObject*
bw_treedict__keys_dec(bw_treedict_t* td) {
	PyObject* ret;

	bw_treedict__rdlock_SET(td, NULL);
	if ((ret = PyList_New(td->td_length)) != NULL) {
		_sA i;
		px_genlink_t*** k;

		k = pxgtc_pathmax(td->td_path, &td->td_root);
		for (i = 0; (k = pxgtc_wprev(td->td_path, k)) != NULL; i++) {
			PyObject* key = ((bw_treedict_node_t*)**k)->n_key;
			Py_INCREF(key);
			PyList_SET_ITEM(ret, i, key);
		}
	}
	bw_treedict__rdlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__values_inc(bw_treedict_t* td) {
	PyObject* ret;

	bw_treedict__rdlock_SET(td, NULL);
	if ((ret = PyList_New(td->td_length)) != NULL) {
		_sA i;
		px_genlink_t*** k;

		k = pxgtc_pathmin(td->td_path, &td->td_root);
		for (i = 0; (k = pxgtc_wnext(td->td_path, k)) != NULL; i++) {
			PyObject* value = ((bw_treedict_node_t*)**k)->n_value;
			Py_INCREF(value);
			PyList_SET_ITEM(ret, i, value);
		}
	}
	bw_treedict__rdlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__values_dec(bw_treedict_t* td) {
	PyObject* ret;

	bw_treedict__rdlock_SET(td, NULL);
	if ((ret = PyList_New(td->td_length)) != NULL) {
		_sA i;
		px_genlink_t*** k;

		k = pxgtc_pathmax(td->td_path, &td->td_root);
		for (i = 0; (k = pxgtc_wprev(td->td_path, k)) != NULL; i++) {
			PyObject* value = ((bw_treedict_node_t*)**k)->n_value;
			Py_INCREF(value);
			PyList_SET_ITEM(ret, i, value);
		}
	}
	bw_treedict__rdlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__items_inc(bw_treedict_t* td) {
	PyObject* ret;

	bw_treedict__rdlock_SET(td, NULL);
	if ((ret = PyList_New(td->td_length)) != NULL) {
		_sA i;
		px_genlink_t*** k;
		bw_treedict_node_t* n;

		k = pxgtc_pathmin(td->td_path, &td->td_root);
		for (i = 0; (k = pxgtc_wnext(td->td_path, k)) != NULL; i++) {
			PyObject* item;
			n = ((bw_treedict_node_t*)**k);
			if ((item = bw_pair_New(n->n_key, n->n_value)) == NULL) {
				Py_DECREF(ret);
				ret = PyErr_NoMemory();
				goto failed;
			}
			PyList_SET_ITEM(ret, i, item);
		}
	}
failed:
	bw_treedict__rdlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__items_dec(bw_treedict_t* td) {
	PyObject* ret;

	bw_treedict__rdlock_SET(td, NULL);
	if ((ret = PyList_New(td->td_length)) != NULL) {
		_sA i;
		px_genlink_t*** k;
		bw_treedict_node_t* n;

		k = pxgtc_pathmax(td->td_path, &td->td_root);
		for (i = 0; (k = pxgtc_wprev(td->td_path, k)) != NULL; i++) {
			PyObject* item;
			n = ((bw_treedict_node_t*)**k);
			if ((item = bw_pair_New(n->n_key, n->n_value)) == NULL) {
				Py_DECREF(ret);
				ret = PyErr_NoMemory();
				goto failed;
			}
			PyList_SET_ITEM(ret, i, item);
		}
	}
failed:
	bw_treedict__rdlock_UNSET(td);
	return ret;
}

static
PyObject*
bw_treedict__copy_inc(bw_treedict_t* td) {
	bw_treedict_t* td2 = NULL;
	pxgtc_algo_t* algo = td->td_algo;
	px_genlink_t*** k = NULL;
	px_genlink_t*** k2 = NULL;
	px_genlink_t*** path;
	px_genlink_t*** path2;

	bw_treedict__rdlock_SET(td, NULL);
	if ((td2 = (bw_treedict_t*)Py_TYPE(td)->tp_alloc(Py_TYPE(td), 0)) == NULL)
		goto failed;
	bw_treedict__default(td2);
	Py_XINCREF(td->td_compare);
	td2->td_compare = td->td_compare;
	td2->td_algo = algo;
	td2->td_length = td->td_length;
	td2->td_lockcount = 0;
	path = td->td_path;
	path2 = td2->td_path;
	k = pxgtc_pathmin(path, &td->td_root);
	while ((k = pxgtc_wnext(path, k)) != NULL) {
		bw_treedict_node_t* n = (bw_treedict_node_t*)**k;
		bw_treedict_node_t* n2;

		k2 = pxgtc_pathmax(path2, &td2->td_root);
		if ((n2 = PyMem_MALLOC(sizeof(*n2))) == NULL) {
			Py_CLEAR(td2);
			PyErr_NoMemory();
			goto failed;
		}
		n2->n_link.link[0] = n2->n_link.link[1] = NULL;
		algo->algo_setstate(algo, &n2->n_link, algo->algo_getstate(algo, &n->n_link));
		Py_INCREF(n->n_key);
		n2->n_key = n->n_key;
		Py_INCREF(n->n_value);
		n2->n_value = n->n_value;
		**k2 = &n2->n_link;
		algo->algo_balance_insert(algo, path2, k2);
	}
failed:
	bw_treedict__rdlock_UNSET(td);
	return (PyObject*)td2;
}

static
PyObject*
bw_treedict__copy_dec(bw_treedict_t* td) {
	bw_treedict_t* td2 = NULL;
	pxgtc_algo_t* algo = td->td_algo;
	px_genlink_t*** k = NULL;
	px_genlink_t*** k2 = NULL;
	px_genlink_t*** path;
	px_genlink_t*** path2;

	bw_treedict__rdlock_SET(td, NULL);
	if ((td2 = (bw_treedict_t*)Py_TYPE(td)->tp_alloc(Py_TYPE(td), 0)) == NULL)
		goto failed;
	bw_treedict__default(td2);
	Py_XINCREF(td->td_compare);
	td2->td_compare = td->td_compare;
	td2->td_algo = algo;
	td2->td_length = td->td_length;
	td2->td_lockcount = 0;
	path = td->td_path;
	path2 = td2->td_path;
	k = pxgtc_pathmax(path, &td->td_root);
	while ((k = pxgtc_wprev(path, k)) != NULL) {
		bw_treedict_node_t* n = (bw_treedict_node_t*)**k;
		bw_treedict_node_t* n2;

		k2 = pxgtc_pathmin(path2, &td2->td_root);
		if ((n2 = PyMem_MALLOC(sizeof(*n2))) == NULL) {
			Py_CLEAR(td2);
			PyErr_NoMemory();
			goto failed;
		}
		n2->n_link.link[0] = n2->n_link.link[1] = NULL;
		algo->algo_setstate(algo, &n2->n_link, algo->algo_getstate(algo, &n->n_link));
		Py_INCREF(n->n_key);
		n2->n_key = n->n_key;
		Py_INCREF(n->n_value);
		n2->n_value = n->n_value;
		**k2 = &n2->n_link;
		algo->algo_balance_insert(algo, path2, k2);
	}
failed:
	bw_treedict__rdlock_UNSET(td);
	return (PyObject*)td2;
}

static
PyObject*
bw_treedict__xcopy_inc(bw_treedict_t* td) {
	bw_treedict_t* td2 = NULL;
	pxgtc_algo_t* algo = td->td_algo;
	px_genlink_t*** k = NULL;
	px_genlink_t*** k2 = NULL;
	px_genlink_t*** path;
	px_genlink_t*** path2;
	bw_treedict_node_t* n;
	bw_treedict_node_t* n2;

	bw_treedict__rdlock_SET(td, NULL);
	if ((td2 = (bw_treedict_t*)Py_TYPE(td)->tp_alloc(Py_TYPE(td), 0)) == NULL)
		goto failed;
	bw_treedict__default(td2);
	Py_XINCREF(td->td_compare);
	td2->td_compare = td->td_compare;
	td2->td_length = td->td_length;
	td2->td_lockcount = 0;
	td2->td_algo = algo;
	if ((n = (bw_treedict_node_t*)td->td_root) == NULL)
		goto ok;
	if ((n2 = PyMem_MALLOC(sizeof(*n))) == NULL) {
		Py_CLEAR(td2);
		PyErr_NoMemory();
		goto failed;
	}
	n2->n_link.link[0] = n2->n_link.link[1] = NULL;
	algo->algo_setstate(algo, &n2->n_link, algo->algo_getstate(algo, &n->n_link));
	Py_INCREF(n->n_key);
	n2->n_key = n->n_key;
	Py_INCREF(n->n_value);
	n2->n_value = n->n_value;

	path = k = td->td_path;
	*path = &td->td_root;

	td2->td_root = &n2->n_link;
	path2 = k2 = td2->td_path;
	*path2 = &td2->td_root;

	for (;;) {
		bw_treedict_node_t* n = (bw_treedict_node_t*)**k;
		bw_treedict_node_t* n2;

		if ((k = pxgtc_xnext(path, k)) == NULL)
			break;
		k2 = pxgtc_xnext(path2, k2);
		if ((n = ((bw_treedict_node_t*)**k)) == NULL)
			continue;
		if ((n2 = PyMem_MALLOC(sizeof(*n2))) == NULL) {
			Py_CLEAR(td2);
			PyErr_NoMemory();
			goto failed;
		}
		n2->n_link.link[0] = n2->n_link.link[1] = NULL;
		algo->algo_setstate(algo, &n2->n_link, algo->algo_getstate(algo, &n->n_link));
		Py_INCREF(n->n_key);
		n2->n_key = n->n_key;
		Py_INCREF(n->n_value);
		n2->n_value = n->n_value;
		**k2 = &n2->n_link;
		algo->algo_setstate(algo, &n2->n_link,
				algo->algo_getstate(algo, &n->n_link));
	}
ok:
failed:
	bw_treedict__rdlock_UNSET(td);
	return (PyObject*)td2;
}

static
PyObject*
bw_treedict__xcopy_dec(bw_treedict_t* td) {
	bw_treedict_t* td2 = NULL;
	pxgtc_algo_t* algo = td->td_algo;
	px_genlink_t*** k = NULL;
	px_genlink_t*** k2 = NULL;
	px_genlink_t*** path;
	px_genlink_t*** path2;
	bw_treedict_node_t* n;
	bw_treedict_node_t* n2;

	bw_treedict__rdlock_SET(td, NULL);
	if ((td2 = (bw_treedict_t*)Py_TYPE(td)->tp_alloc(Py_TYPE(td), 0)) == NULL)
		goto failed;
	bw_treedict__default(td2);
	Py_XINCREF(td->td_compare);
	td2->td_compare = td->td_compare;
	td2->td_length = td->td_length;
	td2->td_lockcount = 0;
	td2->td_algo = algo;
	if ((n = (bw_treedict_node_t*)td->td_root) == NULL)
		goto ok;
	if ((n2 = PyMem_MALLOC(sizeof(*n))) == NULL) {
		Py_CLEAR(td2);
		PyErr_NoMemory();
		goto failed;
	}
	n2->n_link.link[0] = n2->n_link.link[1] = NULL;
	algo->algo_setstate(algo, &n2->n_link, algo->algo_getstate(algo, &n->n_link));
	Py_INCREF(n->n_key);
	n2->n_key = n->n_key;
	Py_INCREF(n->n_value);
	n2->n_value = n->n_value;

	path = k = td->td_path;
	*path = &td->td_root;

	td2->td_root = &n2->n_link;
	path2 = k2 = td2->td_path;
	*path2 = &td2->td_root;

	for (;;) {
		bw_treedict_node_t* n = (bw_treedict_node_t*)**k;
		bw_treedict_node_t* n2;

		if ((k = pxgtc_xprev(path, k)) == NULL)
			break;
		k2 = pxgtc_xprev(path2, k2);
		if ((n = ((bw_treedict_node_t*)**k)) == NULL)
			continue;
		if ((n2 = PyMem_MALLOC(sizeof(*n2))) == NULL) {
			Py_CLEAR(td2);
			PyErr_NoMemory();
			goto failed;
		}
		n2->n_link.link[0] = n2->n_link.link[1] = NULL;
		algo->algo_setstate(algo, &n2->n_link, algo->algo_getstate(algo, &n->n_link));
		Py_INCREF(n->n_key);
		n2->n_key = n->n_key;
		Py_INCREF(n->n_value);
		n2->n_value = n->n_value;
		**k2 = &n2->n_link;
		algo->algo_setstate(algo, &n2->n_link,
				algo->algo_getstate(algo, &n->n_link));
	}
ok:
failed:
	bw_treedict__rdlock_UNSET(td);
	return (PyObject*)td2;
}

static
PyObject*
bw_treedict__clear_inc(bw_treedict_t* td) {
	PyObject* ret = Py_None;
	px_genlink_t*** k;
	px_genlink_t*** path = td->td_path;

	bw_treedict__wrlock_SET(td, NULL);
	k = pxgtc_pathmin(path, &td->td_root);
	while ((k = pxgtc_znext(path, k)) != NULL) {
		bw_treedict_node_t* n = (bw_treedict_node_t*)**k;
		Py_DECREF(n->n_key);
		Py_DECREF(n->n_value);
		PyMem_FREE(n);
	}
	td->td_root = NULL;
	td->td_length = 0;
	bw_treedict__wrlock_UNSET(td);
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_treedict__clear_dec(bw_treedict_t* td) {
	PyObject* ret = Py_None;
	px_genlink_t*** k;
	px_genlink_t*** path = td->td_path;

	bw_treedict__wrlock_SET(td, NULL);
	k = pxgtc_pathmax(path, &td->td_root);
	while ((k = pxgtc_zprev(path, k)) != NULL) {
		bw_treedict_node_t* n = (bw_treedict_node_t*)**k;
		Py_DECREF(n->n_key);
		Py_DECREF(n->n_value);
		PyMem_FREE(n);
	}
	td->td_root = NULL;
	td->td_length = 0;
	bw_treedict__wrlock_UNSET(td);
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_treedict__xclear_inc(bw_treedict_t* td) {
	px_genlink_t link;
	px_genlink_t* p;

	bw_treedict__wrlock_SET(td, NULL);
	pxgtc2qcl_prev(td->td_path, &td->td_root, &link);
	while ((p = link.link[0]) != &link) {
		bw_treedict_node_t* n = (bw_treedict_node_t*)p;

		PxQCL_selfremove(p, 0);
		Py_DECREF(n->n_key);
		Py_DECREF(n->n_value);
		PyMem_FREE(n);
	}
	td->td_length = 0;
	td->td_root = NULL;
	bw_treedict__wrlock_UNSET(td);
	Py_INCREF(Py_None);
	return Py_None;
}

static
PyObject*
bw_treedict__xclear_dec(bw_treedict_t* td) {
	px_genlink_t link;
	px_genlink_t* p;

	bw_treedict__wrlock_SET(td, NULL);
	pxgtc2qcl_next(td->td_path, &td->td_root, &link);
	while ((p = link.link[0]) != &link) {
		bw_treedict_node_t* n = (bw_treedict_node_t*)p;
	
		PxQCL_selfremove(p, 1);
		Py_DECREF(n->n_key);
		Py_DECREF(n->n_value);
		PyMem_FREE(n);
	}
	td->td_length = 0;
	td->td_root = NULL;
	bw_treedict__wrlock_UNSET(td);
	Py_INCREF(Py_None);
	return Py_None;
}

static
PyObject*
bw_treedict__update(bw_treedict_t* td, PyObject* arg) {
	PyObject* ret = NULL;
	PyObject* key = NULL;
	PyObject* value = NULL;
	PyObject* iter = NULL;
	PyObject* seq2 = NULL;
	PyObject* next = NULL;
	pxgtc_algo_t* algo = td->td_algo;
	px_genlink_t*** k = NULL;
	px_genlink_t*** path = td->td_path;
	bw_treedict_node_t* n;

	bw_treedict__wrlock_SET(td, NULL);
	if (Py_TYPE(arg) == &PyDict_Type) {
		Py_ssize_t pos = 0;
		for (;;) {
			if (!PyDict_Next(arg, &pos, &key, &value)) {
				if (PyErr_Occurred())
					goto failed;
				ret = Py_None;
				Py_INCREF(ret);
				goto ok;
			}
			if ((k = bw_treedict__getbypath(td, td->td_path, key)) == NULL)
				goto failed;
			if ((n = (bw_treedict_node_t*)**k) == NULL) { 
				if ((n = PyMem_MALLOC(sizeof(*n))) == NULL) {
					PyErr_NoMemory();
					goto failed;
				}
				//n->n_link.link[0] = n->n_link.link[1] = NULL;
				//algo->algo_setstate(algo, n, 0);
				Py_INCREF(key);
				n->n_key = key;
				Py_INCREF(value);
				n->n_value = value;
				**k = &n->n_link;
				td->td_length++;
				algo->algo_balance_insert(algo, path, k);
			} else {
				Py_INCREF(value);
				Py_DECREF(n->n_value);
				n->n_value = value;
			}
		}
	}

	if ((iter = PyObject_GetIter(arg)) == NULL)
		goto failed;
	for (;;) {
		if ((next = PyIter_Next(iter)) == NULL) {
			if (PyErr_Occurred())
				goto failed2;
			Py_DECREF(iter);
			ret = Py_None;
			Py_INCREF(ret);
			goto ok;
		}
		if ((seq2 = PyObject_GetIter(next)) == NULL) 
			goto failed2;
		Py_CLEAR(next);
		if ((key = PyIter_Next(seq2)) == NULL) {
			if (PyErr_Occurred())
				goto failed2;
			break;
		}
		if ((value = PyIter_Next(seq2)) == NULL) {
			if (PyErr_Occurred())
				goto failed2;
			break;
		}
		if ((next = PyIter_Next(seq2)) != NULL) {
			PyErr_SetString(PyExc_ValueError, "treedict update sequence element; 2 is required");
			goto failed2;
		}
		if (PyErr_Occurred())
			goto failed2;
		Py_CLEAR(seq2);
		if ((k = bw_treedict__getbypath(td, td->td_path, key)) == NULL)
			goto failed2;
		if ((n = (bw_treedict_node_t*)**k) == NULL) { 
			if ((n = PyMem_MALLOC(sizeof(*n))) == NULL) {
				PyErr_NoMemory();
				goto failed2;
			}
			//n->n_link.link[0] = n->n_link.link[1] = NULL;
			//algo->algo_setstate(algo, n, 0);
			// borrowed refcount
			n->n_key = key;
			// borrowed refcount
			n->n_value = value;
			**k = &n->n_link;
			td->td_length++;
			algo->algo_balance_insert(algo, path, k);

		} else {
			Py_DECREF(key);
			Py_DECREF(n->n_value);
			// borrowed refcount
			n->n_value = value;
		}
		key = value = NULL;
	}
failed2:
	Py_XDECREF(key);
	Py_XDECREF(value);
	Py_XDECREF(iter);
	Py_XDECREF(seq2);
	Py_XDECREF(next);
ok:
failed:
	bw_treedict__wrlock_UNSET(td);
	return (PyObject*)ret;
}

static
PyObject*
bw_treedict__sq_contains(bw_treedict_t* td, PyObject* key) {
	PyObject* ret = NULL;
	bw_treedict_node_t* n;

	bw_treedict__rdlock_SET(td, NULL);
	if ((n = bw_treedict__getbykey(td, key)) != NULL)
		ret = Py_True;
	else
	if (PyErr_Occurred())
		goto failed;
	else
		ret = Py_False;
	Py_INCREF(ret);
failed:
	bw_treedict__rdlock_UNSET(td);
	return ret;
}

static
Py_ssize_t
bw_treedict__mp_length(bw_treedict_t* td) {
	return (Py_ssize_t)td->td_length;
}

static
PyObject*
bw_treedict__mp_subscript(bw_treedict_t* td, PyObject* key) {
	PyObject* ret = NULL;
	bw_treedict_node_t* n;

	bw_treedict__rdlock_SET(td, NULL);
	if ((n = bw_treedict__getbykey(td, key)) != NULL)
		ret = n->n_value;
	else
	if (PyErr_Occurred())
		goto failed;
	else {
		PyErr_SetObject(PyExc_KeyError, key);
		goto failed;
	}
	Py_INCREF(ret);
failed:
	bw_treedict__rdlock_UNSET(td);
	return ret;
}

static
int
bw_treedict__mp_ass_subscript(bw_treedict_t* td, PyObject* key, PyObject* value) {
	int ret = -1;
	pxgtc_algo_t* algo = td->td_algo;
	px_genlink_t*** k;
	bw_treedict_node_t* n;

	bw_treedict__wrlock_SET(td, -1);
	if ((k = bw_treedict__getbypath(td, td->td_path, key)) == NULL)
		goto failed;
	if ((n = (bw_treedict_node_t*)**k) != NULL) {
		if (value == NULL) {
			Py_DECREF(n->n_key);
			Py_DECREF(n->n_value);
			td->td_length--;
			algo->algo_balance_remove(algo, td->td_path, k);
			PyMem_FREE(n);
		} else {
			Py_INCREF(value);
			Py_DECREF(n->n_value);
			n->n_value = value;
		}
	} else {
		if (value == NULL) {
			PyErr_SetObject(PyExc_KeyError, key);
			goto failed;
		} else {
			if ((n = PyMem_MALLOC(sizeof(*n))) == NULL) {
				PyErr_NoMemory();
				goto failed;
			}
			//n->n_link.link[0] = n->n_link.link[1] = NULL;
			//algo->algo_setstate(algo, n, 0);
			Py_INCREF(key);
			n->n_key = key;
			Py_INCREF(value);
			n->n_value = value;
			**k = &n->n_link;
			td->td_length++;
			algo->algo_balance_insert(algo, td->td_path, k);
		}
	}
	ret = 0;
failed:
	bw_treedict__wrlock_UNSET(td);
	return ret;

}
static PyObject* bw_treedict__iter_keys_inc(bw_treedict_t* td);
static PyObject* bw_treedict__iter_keys_dec(bw_treedict_t* td);
static PyObject* bw_treedict__iter_values_inc(bw_treedict_t* td);
static PyObject* bw_treedict__iter_values_dec(bw_treedict_t* td);
static PyObject* bw_treedict__iter_items_inc(bw_treedict_t* td);
static PyObject* bw_treedict__iter_items_dec(bw_treedict_t* td);
static PyObject* bw_treedict__iter_keys_inc_gt(bw_treedict_t* td, PyObject* gt);
static PyObject* bw_treedict__iter_keys_dec_lt(bw_treedict_t* td, PyObject* gt);
static PyObject* bw_treedict__iter_values_inc_gt(bw_treedict_t* td, PyObject* gt);
static PyObject* bw_treedict__iter_values_dec_lt(bw_treedict_t* td, PyObject* gt);
static PyObject* bw_treedict__iter_items_inc_gt(bw_treedict_t* td, PyObject* gt);
static PyObject* bw_treedict__iter_items_dec_lt(bw_treedict_t* td, PyObject* gt);
static PyObject* bw_treedict__iter_z_keys_inc(bw_treedict_t* td);
static PyObject* bw_treedict__iter_z_keys_dec(bw_treedict_t* td);
static PyObject* bw_treedict__iter_z_values_inc(bw_treedict_t* td);
static PyObject* bw_treedict__iter_z_values_dec(bw_treedict_t* td);
static PyObject* bw_treedict__iter_z_items_inc(bw_treedict_t* td);
static PyObject* bw_treedict__iter_z_items_dec(bw_treedict_t* td);
static PyObject* bw_treedict__iter_v_keys_inc(bw_treedict_t* td);
static PyObject* bw_treedict__iter_v_keys_dec(bw_treedict_t* td);
static PyObject* bw_treedict__iter_v_values_inc(bw_treedict_t* td);
static PyObject* bw_treedict__iter_v_values_dec(bw_treedict_t* td);
static PyObject* bw_treedict__iter_v_items_inc(bw_treedict_t* td);
static PyObject* bw_treedict__iter_v_items_dec(bw_treedict_t* td);

static PyMethodDef bw_treedict__tp_methods[] = {
{ "get", (PyCFunction)bw_treedict__get, METH_VARARGS,
	""/*doc_bw_treedict__get*/},
{ "setdefault", (PyCFunction)bw_treedict__setdefault, METH_VARARGS,
	""/*doc_bw_treedict__setdefault*/},
{ "replace", (PyCFunction)bw_treedict__replace, METH_VARARGS,
	""/*doc_bw_treedict__replace*/},
{ "pop", (PyCFunction)bw_treedict__pop, METH_VARARGS,
	""/*doc_bw_treedict__pop*/},
{ "popmin", (PyCFunction)bw_treedict__popmin, METH_NOARGS,
	""/*doc_bw_treedict__popmin*/},
{ "popmax", (PyCFunction)bw_treedict__popmax, METH_NOARGS,
	""/*doc_bw_treedict__popmax*/},
{ "poppairmin", (PyCFunction)bw_treedict__poppairmin, METH_NOARGS,
	""/*doc_bw_treedict__poppairmin*/},
{ "poppairmax", (PyCFunction)bw_treedict__poppairmax, METH_NOARGS,
	""/*doc_bw_treedict__poppairmax*/},
{ "popitem", (PyCFunction)bw_treedict__poppairmin, METH_NOARGS,
	""/*doc_bw_treedict__poppairmin*/},
{ "pushmin", (PyCFunction)bw_treedict__pushmin, METH_VARARGS,
	""/*doc_bw_treedict__pushmin*/},
{ "pushmax", (PyCFunction)bw_treedict__pushmax, METH_VARARGS,
	""/*doc_bw_treedict__pushmax*/},
{ "keys", (PyCFunction)bw_treedict__keys_inc, METH_NOARGS,
	""/*doc_bw_treedict__keys_inc*/},
{ "keys_inc", (PyCFunction)bw_treedict__keys_inc, METH_NOARGS,
	""/*doc_bw_treedict__keys_inc*/},
{ "keys_dec", (PyCFunction)bw_treedict__keys_dec, METH_NOARGS,
	""/*doc_bw_treedict__keys_dec*/},
{ "values", (PyCFunction)bw_treedict__values_inc, METH_NOARGS,
	""/*doc_bw_treedict__values_inc*/},
{ "values_inc", (PyCFunction)bw_treedict__values_inc, METH_NOARGS,
	""/*doc_bw_treedict__values_inc*/},
{ "values_dec", (PyCFunction)bw_treedict__values_dec, METH_NOARGS,
	""/*doc_bw_treedict__values_dec*/},
{ "items", (PyCFunction)bw_treedict__items_inc, METH_NOARGS,
	""/*doc_bw_treedict__items_inc*/},
{ "items_inc", (PyCFunction)bw_treedict__items_inc, METH_NOARGS,
	""/*doc_bw_treedict__items_inc*/},
{ "items_dec", (PyCFunction)bw_treedict__items_dec, METH_NOARGS,
	""/*doc_bw_treedict__items_dec*/},
{ "iter_keys", (PyCFunction)bw_treedict__iter_keys_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_keys_inc*/},
{ "iter_keys_inc", (PyCFunction)bw_treedict__iter_keys_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_keys_inc*/},
{ "iter_keys_dec", (PyCFunction)bw_treedict__iter_keys_dec, METH_NOARGS,
	""/*doc_bw_treedict__iter_keys_dec*/},
{ "iter_values", (PyCFunction)bw_treedict__iter_values_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_values_inc*/},
{ "iter_values_inc", (PyCFunction)bw_treedict__iter_values_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_values_inc*/},
{ "iter_values_dec", (PyCFunction)bw_treedict__iter_values_dec, METH_NOARGS,
	""/*doc_bw_treedict__iter_values_dec*/},
{ "iter_items", (PyCFunction)bw_treedict__iter_items_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_items_inc*/},
{ "iter_items_inc", (PyCFunction)bw_treedict__iter_items_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_items_inc*/},
{ "iter_items_dec", (PyCFunction)bw_treedict__iter_items_dec, METH_NOARGS,
	""/*doc_bw_treedict__iter_items_dec*/},
{ "iter_keys_inc_gt", (PyCFunction)bw_treedict__iter_keys_inc_gt, METH_O,
	""/*doc_bw_treedict__iter_keys_inc_gt*/},
{ "iter_keys_dec_lt", (PyCFunction)bw_treedict__iter_keys_dec_lt, METH_O,
	""/*doc_bw_treedict__iter_keys_dec_lt*/},
{ "iter_values_inc_gt", (PyCFunction)bw_treedict__iter_values_inc_gt, METH_O,
	""/*doc_bw_treedict__iter_values_inc_gt*/},
{ "iter_values_dec_lt", (PyCFunction)bw_treedict__iter_values_dec_lt, METH_O,
	""/*doc_bw_treedict__iter_values_dec_lt*/},
{ "iter_items_inc_gt", (PyCFunction)bw_treedict__iter_items_inc_gt, METH_O,
	""/*doc_bw_treedict__iter_items_inc_gt*/},
{ "iter_items_dec_lt", (PyCFunction)bw_treedict__iter_items_dec_lt, METH_O,
	""/*doc_bw_treedict__iter_items_dec_lt*/},
{ "iter_z_keys", (PyCFunction)bw_treedict__iter_z_keys_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_z_keys_inc*/},
{ "iter_z_keys_inc", (PyCFunction)bw_treedict__iter_z_keys_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_z_keys_inc*/},
{ "iter_z_keys_dec", (PyCFunction)bw_treedict__iter_z_keys_dec, METH_NOARGS,
	""/*doc_bw_treedict__iter_z_keys_dec*/},
{ "iter_z_values", (PyCFunction)bw_treedict__iter_z_values_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_z_values_inc*/},
{ "iter_z_values_inc", (PyCFunction)bw_treedict__iter_z_values_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_z_values_inc*/},
{ "iter_z_values_dec", (PyCFunction)bw_treedict__iter_z_values_dec, METH_NOARGS,
	""/*doc_bw_treedict__iter_z_values_dec*/},
{ "iter_z_items", (PyCFunction)bw_treedict__iter_z_items_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_z_items_inc*/},
{ "iter_z_items_inc", (PyCFunction)bw_treedict__iter_z_items_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_z_items_inc*/},
{ "iter_z_items_dec", (PyCFunction)bw_treedict__iter_z_items_dec, METH_NOARGS,
	""/*doc_bw_treedict__iter_z_items_dec*/},
{ "iter_v_keys", (PyCFunction)bw_treedict__iter_v_keys_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_v_keys_inc*/},
{ "iter_v_keys_inc", (PyCFunction)bw_treedict__iter_v_keys_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_v_keys_inc*/},
{ "iter_v_keys_dec", (PyCFunction)bw_treedict__iter_v_keys_dec, METH_NOARGS,
	""/*doc_bw_treedict__iter_v_keys_dec*/},
{ "iter_v_values", (PyCFunction)bw_treedict__iter_v_values_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_v_values_inc*/},
{ "iter_v_values_inc", (PyCFunction)bw_treedict__iter_v_values_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_v_values_inc*/},
{ "iter_v_values_dec", (PyCFunction)bw_treedict__iter_v_values_dec, METH_NOARGS,
	""/*doc_bw_treedict__iter_v_values_dec*/},
{ "iter_v_items", (PyCFunction)bw_treedict__iter_v_items_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_v_items_inc*/},
{ "iter_v_items_inc", (PyCFunction)bw_treedict__iter_v_items_inc, METH_NOARGS,
	""/*doc_bw_treedict__iter_v_items_inc*/},
{ "iter_v_items_dec", (PyCFunction)bw_treedict__iter_v_items_dec, METH_NOARGS,
	""/*doc_bw_treedict__iter_v_items_dec*/},
{ "copy", (PyCFunction)bw_treedict__copy_inc, METH_NOARGS,
	""/*doc_bw_treedict__copy_inc*/},
{ "copy_inc", (PyCFunction)bw_treedict__copy_inc, METH_NOARGS,
	""/*doc_bw_treedict__copy_inc*/},
{ "copy_dec", (PyCFunction)bw_treedict__copy_dec, METH_NOARGS,
	""/*doc_bw_treedict__copy_dec*/},
{ "xcopy", (PyCFunction)bw_treedict__xcopy_inc, METH_NOARGS,
	""/*doc_bw_treedict__xcopy_inc*/},
{ "xcopy_inc", (PyCFunction)bw_treedict__xcopy_inc, METH_NOARGS,
	""/*doc_bw_treedict__xcopy_inc*/},
{ "xcopy_dec", (PyCFunction)bw_treedict__xcopy_dec, METH_NOARGS,
	""/*doc_bw_treedict__xcopy_dec*/},
{ "clear", (PyCFunction)bw_treedict__clear_inc, METH_NOARGS,
	""/*doc_bw_treedict__clear_inc*/},
{ "clear_inc", (PyCFunction)bw_treedict__clear_inc, METH_NOARGS,
	""/*doc_bw_treedict__clear_inc*/},
{ "clear_dec", (PyCFunction)bw_treedict__clear_dec, METH_NOARGS,
	""/*doc_bw_treedict__clear_dec*/},
{ "xclear", (PyCFunction)bw_treedict__xclear_inc, METH_NOARGS,
	""/*doc_bw_treedict__xclear_inc*/},
{ "xclear_inc", (PyCFunction)bw_treedict__xclear_inc, METH_NOARGS,
	""/*doc_bw_treedict__xclear_inc*/},
{ "xclear_dec", (PyCFunction)bw_treedict__xclear_dec, METH_NOARGS,
	""/*doc_bw_treedict__xclear_dec*/},
{ "update", (PyCFunction)bw_treedict__update, METH_O,
	""/*doc_bw_treedict__update*/},
{ "fromkeys", (PyCFunction)bw_treedict__fromkeys, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
	""/*doc_bw_treedict__fromkeys*/},
{ NULL, NULL, 0, NULL },
};


static
PySequenceMethods
bw_treedict__tp_as_sequence = {
	.sq_length = NULL,
	.sq_concat = NULL,
	.sq_repeat = NULL,
	.sq_item = NULL,
	.sq_ass_item = NULL,
	.sq_contains = (objobjproc)bw_treedict__sq_contains,
	.sq_inplace_concat = NULL,
	.sq_inplace_repeat = NULL
};

static
PyMappingMethods
bw_treedict__tp_as_mapping = {
	.mp_length = (lenfunc)bw_treedict__mp_length,
	.mp_subscript = (binaryfunc)bw_treedict__mp_subscript,
	.mp_ass_subscript = (objobjargproc)bw_treedict__mp_ass_subscript,
};

PyTypeObject
bw_treedict_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "billow.treedict",
	.tp_basicsize = sizeof(bw_treedict_t),
	.tp_dealloc = (destructor)bw_treedict__tp_dealloc,
	.tp_repr = (reprfunc)bw_treedict__tp_repr,
	.tp_as_sequence = (PySequenceMethods*)&bw_treedict__tp_as_sequence,
	.tp_as_mapping = (PyMappingMethods*)&bw_treedict__tp_as_mapping,
	.tp_getattro = (getattrofunc)PyObject_GenericGetAttr,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
	//.tp_doc = bw_treedict__doc,
	.tp_traverse = (traverseproc)bw_treedict__tp_traverse,
	.tp_clear = (inquiry)bw_treedict__tp_clear,
	.tp_weaklistoffset = 0,
	.tp_methods = (struct PyMethodDef*)bw_treedict__tp_methods,
	.tp_new = (newfunc)bw_treedict__tp_new,
};

static
_sA
bw_treedict_iter__tp_traverse(bw_treedict_iter_t* it, visitproc visit, void* arg) {
	return visit((PyObject*)it->it_td, arg);
}

static
_sA
bw_treedict_iter__tp_clear(bw_treedict_iter_t* it) {
	bw_treedict__rdlock_UNSET(it->it_td);
	Py_CLEAR(it->it_td);
	return 0;
}

static
void
bw_treedict_iter__tp_dealloc(bw_treedict_iter_t* it) {
	PyObject_GC_UnTrack((PyObject*)it);
#if PY_VERSION_HEX < 0x03080000
	Py_TRASHCAN_SAFE_BEGIN(it)
#else /* PY_VERSION_HEX < 0x03080000 */
	Py_TRASHCAN_BEGIN(it, bw_treedict_iter__tp_dealloc)
#endif /* PY_VERSION_HEX < 0x03080000 */
	bw_treedict_iter__tp_clear(it);
	PyObject_GC_Del(it);
#if PY_VERSION_HEX < 0x03080000
	Py_TRASHCAN_SAFE_END(it)
#else /* PY_VERSION_HEX < 0x03080000 */
	Py_TRASHCAN_END
#endif /* PY_VERSION_HEX < 0x03080000 */
}

static
PyObject*
bw_treedict_iter__tp_iternext(bw_treedict_iter_t* it) {
	px_genlink_t*** k;
	bw_treedict_node_t* n;

	k = it->it_step(it->it_path, it->it_k);
	if (k == NULL)
		return NULL;
	if ((n = (bw_treedict_node_t*)**k) == NULL)
		return NULL;
	it->it_k = k;
	return it->it_getitem(n);
}

static
PyObject*
bw_treedict_iter__getitem_key(bw_treedict_node_t* n) {
	Py_INCREF(n->n_key);
	return n->n_key;
}

static
PyObject*
bw_treedict_iter__getitem_value(bw_treedict_node_t* n) {
	Py_INCREF(n->n_value);
	return n->n_value;
}

static
PyObject*
bw_treedict_iter__getitem_item(bw_treedict_node_t* n) {
	return bw_pair_New(n->n_key, n->n_value);
}

static
PyObject*
bw_treedict__iter_keys_inc(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_wnext;
	it->it_getitem = bw_treedict_iter__getitem_key;
	it->it_k = pxgtc_pathmin(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_keys_dec(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_wprev;
	it->it_getitem = bw_treedict_iter__getitem_key;
	it->it_k = pxgtc_pathmax(it->it_path, &td->td_root);
	return (PyObject*)it;
}
static
PyObject*
bw_treedict__iter_values_inc(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_wnext;
	it->it_getitem = bw_treedict_iter__getitem_value;
	it->it_k = pxgtc_pathmin(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_values_dec(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_wprev;
	it->it_getitem = bw_treedict_iter__getitem_value;
	it->it_k = pxgtc_pathmax(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_items_inc(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_wnext;
	it->it_getitem = bw_treedict_iter__getitem_item;
	it->it_k = pxgtc_pathmin(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_items_dec(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_wprev;
	it->it_getitem = bw_treedict_iter__getitem_item;
	it->it_k = pxgtc_pathmax(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_keys_inc_gt(bw_treedict_t* td, PyObject* gt) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_wnext;
	it->it_getitem = bw_treedict_iter__getitem_key;
	it->it_k = bw_treedict__getbypath(td, it->it_path, gt);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_keys_dec_lt(bw_treedict_t* td, PyObject* lt) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_wprev;
	it->it_getitem = bw_treedict_iter__getitem_key;
	it->it_k = bw_treedict__getbypath(td, it->it_path, lt);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_values_inc_gt(bw_treedict_t* td, PyObject* gt) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_wnext;
	it->it_getitem = bw_treedict_iter__getitem_value;
	it->it_k = bw_treedict__getbypath(td, it->it_path, gt);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_values_dec_lt(bw_treedict_t* td, PyObject* lt) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_wprev;
	it->it_getitem = bw_treedict_iter__getitem_value;
	it->it_k = bw_treedict__getbypath(td, it->it_path, lt);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_items_inc_gt(bw_treedict_t* td, PyObject* gt) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_wnext;
	it->it_getitem = bw_treedict_iter__getitem_item;
	it->it_k = bw_treedict__getbypath(td, it->it_path, gt);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_items_dec_lt(bw_treedict_t* td, PyObject* lt) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_wprev;
	it->it_getitem = bw_treedict_iter__getitem_item;
	it->it_k = bw_treedict__getbypath(td, it->it_path, lt);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_z_keys_inc(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_znext;
	it->it_getitem = bw_treedict_iter__getitem_key;
	it->it_k = pxgtc_pathmin(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_z_keys_dec(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_zprev;
	it->it_getitem = bw_treedict_iter__getitem_key;
	it->it_k = pxgtc_pathmax(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_z_values_inc(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_znext;
	it->it_getitem = bw_treedict_iter__getitem_value;
	it->it_k = pxgtc_pathmin(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_z_values_dec(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_zprev;
	it->it_getitem = bw_treedict_iter__getitem_value;
	it->it_k = pxgtc_pathmax(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_z_items_inc(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_znext;
	it->it_getitem = bw_treedict_iter__getitem_item;
	it->it_k = pxgtc_pathmin(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_z_items_dec(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_zprev;
	it->it_getitem = bw_treedict_iter__getitem_item;
	it->it_k = pxgtc_pathmax(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_v_keys_inc(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_vnext;
	it->it_getitem = bw_treedict_iter__getitem_key;
	it->it_k = pxgtc_pathmin(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_v_keys_dec(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_vprev;
	it->it_getitem = bw_treedict_iter__getitem_key;
	it->it_k = pxgtc_pathmax(it->it_path, &td->td_root);
	return (PyObject*)it;
}
static
PyObject*
bw_treedict__iter_v_values_inc(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_vnext;
	it->it_getitem = bw_treedict_iter__getitem_value;
	it->it_k = pxgtc_pathmin(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_v_values_dec(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_vprev;
	it->it_getitem = bw_treedict_iter__getitem_value;
	it->it_k = pxgtc_pathmax(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_v_items_inc(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_vnext;
	it->it_getitem = bw_treedict_iter__getitem_item;
	it->it_k = pxgtc_pathmin(it->it_path, &td->td_root);
	return (PyObject*)it;
}

static
PyObject*
bw_treedict__iter_v_items_dec(bw_treedict_t* td) {
	bw_treedict_iter_t* it;

	bw_treedict__rdlock_SET(td, NULL);
	if ((it = (bw_treedict_iter_t*)bw_treedict_iter_Type.tp_alloc(&bw_treedict_iter_Type, 0)) == NULL) {
		bw_treedict__rdlock_UNSET(td);
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(td);
	it->it_td = td;
	it->it_step = pxgtc_vprev;
	it->it_getitem = bw_treedict_iter__getitem_item;
	it->it_k = pxgtc_pathmax(it->it_path, &td->td_root);
	return (PyObject*)it;
}

PyTypeObject
bw_treedict_iter_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "bw_treedict.iterator",
	.tp_basicsize = sizeof(bw_treedict_iter_t),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor)bw_treedict_iter__tp_dealloc,
	.tp_getattro = (getattrofunc)PyObject_GenericGetAttr,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
	.tp_doc = NULL,
	.tp_traverse = (traverseproc)bw_treedict_iter__tp_traverse,
	.tp_clear = NULL,
	.tp_iter = (getiterfunc)PyObject_SelfIter,
	.tp_iternext = (iternextfunc)bw_treedict_iter__tp_iternext,
};

PyObject*
bw_cmp_buffer(PyObject* self, PyObject* args) {
	PyBufferProcs* buffer;
	PyObject* left;
	PyObject* right;
	Py_ssize_t left_size;
	Py_ssize_t right_size;
	Py_buffer left_bytes;
	Py_buffer right_bytes;
	Py_ssize_t minsize;
	long cmp;

	if (!PyArg_UnpackTuple(args, "cmp_buffer", 2, 2, &left, &right))
		return NULL;

	buffer = Py_TYPE(left)->tp_as_buffer;
	if (buffer == NULL
	 || buffer->bf_getbuffer == NULL
	 || buffer->bf_getbuffer(left, &left_bytes, PyBUF_SIMPLE) < 0) {
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	left_size = left_bytes.len;

	buffer = Py_TYPE(right)->tp_as_buffer;
	if (buffer == NULL
	 || buffer->bf_getbuffer == NULL
	 || buffer->bf_getbuffer(right, &right_bytes, PyBUF_SIMPLE) < 0) {
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	right_size = right_bytes.len;

	minsize = (left_size < right_size) ? left_size : right_size;
	if ((cmp = memcmp(left_bytes.buf, right_bytes.buf, minsize)) == 0) {
		if (left_size != right_size)
			left_size < right_size ? --cmp : ++cmp;
	} else
		cmp = cmp < 0 ? -1 : +1;
	return PyInt_FromLong(cmp);
}

#if PY_VERSION_HEX >= 0x03000000
PyObject*
bw_cmp_unicode(PyObject* self, PyObject* args) {
	PyObject* left;
	PyObject* right;
	const char* left_str;
	const char* right_str;
	Py_ssize_t left_size;
	Py_ssize_t right_size;
	Py_ssize_t minsize;
	long cmp;

	if (!PyArg_UnpackTuple(args, "cmp_unicode", 2, 2, &left, &right))
		return NULL;
        if ((left_str = PyUnicode_AsUTF8AndSize(left, &left_size)) == NULL
         || (right_str = PyUnicode_AsUTF8AndSize(right, &right_size)) == NULL) {
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	minsize = (left_size < right_size) ? left_size : right_size;
	if ((cmp = memcmp(left_str, right_str, minsize)) == 0) {
		if (left_size != right_size)
			left_size < right_size ? --cmp : ++cmp;
	} else
		cmp = cmp < 0 ? -1 : +1;
	return PyInt_FromLong(cmp);
}

PyObject*
bw_cmp_long(PyObject* self, PyObject* args) {
	PyLongObject* left;
	PyLongObject* right;
	Py_ssize_t sign;

	if (!PyArg_UnpackTuple(args, "cmp_buffer", 2, 2, &left, &right))
		return NULL;
	if ((!PyInt_Check(left)) || (!PyInt_Check(right))) {
		PyErr_SetString(PyExc_TypeError,
				"compare function should return an int()");
		return NULL;
	}
	if ((sign = Py_SIZE(left) - Py_SIZE(right)) == 0) {
		Py_ssize_t i = Py_ABS(Py_SIZE(left));
		sdigit diff = 0;
		while (--i >= 0) {
#if PY_VERSION_HEX >= 0x030c0000
			diff = (sdigit)left->long_value.ob_digit[i] - (sdigit)right->long_value.ob_digit[i];
#else /* PY_VERSION_HEX >= 0x030c0000 */
			diff = (sdigit)left->ob_digit[i] - (sdigit)right->ob_digit[i];
#endif /* PY_VERSION_HEX >= 0x030c0000 */
			if (diff != 0)
				goto result_ne;

		}
		return PyInt_FromLong(0);
	result_ne:
		sign = Py_SIZE(left) < 0 ? -diff : diff;
		return PyInt_FromLong((sign < 0) ? -1 : +1);
	}
	return PyInt_FromLong((sign < 0) ? -1 : +1);
}
#endif /* PY_VERSION_HEX >= 0x03000000 */
