
/*
 * (c) Vladimir Yu. Stepanov 2010-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#include <Python.h>

#include "px_types.h"
#include "pycompat.h"
#include "billow.h"
#include "bw_debug.h"
#include "bw_flist.h"

#define bw_flist__wrlock_CHECK(x, code)					\
	do {								\
		if ((x)->flist_locked != 0) {				\
			if ((x)->flist_locked > 0) {			\
				if ((x)->flist_frozenlock)		\
					PyErr_SetNone(BwExc_RDLockError);\
				else					\
					PyErr_SetNone(BwExc_FrozenLockError);\
			} else						\
				PyErr_SetNone(BwExc_WRLockError);	\
			return (code);					\
		}							\
	} while (0)

#define bw_flist__rdlock_CHECK(x, code)				\
	do {								\
		if ((x)->flist_locked < 0) {				\
			PyErr_SetNone(BwExc_WRLockError);		\
			return code;					\
		}							\
	} while (0)

#define bw_flist__rdlock_SET(x, code)					\
	do {								\
		bw_flist__rdlock_CHECK(x, code);			\
		(x)->flist_locked += 1;					\
	} while (0)

#define bw_flist__rdlock_UNSET(x)					\
	do {								\
		(x)->flist_locked -= 1;					\
	} while (0)

static
PyObject**
bw_flist__getref(bw_flist* flist, _s6 item) {
	void** pptr = (void**)&flist->flist_rootptr;
	_u3 shiftlevel = flist->flist_shiftlevel;
	_u3 linesize_p2 = flist->flist_linesize_p2;
	_s6 index;
	while (shiftlevel > 0) {
		shiftlevel -= linesize_p2;
		index = item>>shiftlevel;
		item ^= index<<shiftlevel;
		pptr = &((void**)*pptr)[index];
	}
	return (PyObject**)pptr;
}

static
int
bw_flist__clear(bw_flist* flist) {
#if 0
	while (flist->flist_cpos >= 0)
		bw_flist_tailcut(flist);
#endif
	//bw_flist_precached_free(flist, 0);
	return 0;
}

static
void
bw_flist__dealloc(bw_flist* flist) {
	PyObject_GC_UnTrack(flist);
#if PY_VERSION_HEX < 0x03080000
	Py_TRASHCAN_SAFE_BEGIN(flist)
#else /* PY_VERSION_HEX < 0x03080000 */
	Py_TRASHCAN_BEGIN(flist, bw_flist__dealloc)
#endif /* PY_VERSION_HEX < 0x03080000 */
	//while (flist->flist_cpos >= 0)
	//	bw_flist_tailcut(flist);
	//bw_flist_precached_free(flist, 0);
	Py_TYPE(flist)->tp_free(flist);
#if PY_VERSION_HEX < 0x03080000
	Py_TRASHCAN_SAFE_END(flist)
#else /* PY_VERSION_HEX < 0x03080000 */
    	Py_TRASHCAN_END
#endif /* PY_VERSION_HEX < 0x03080000 */
}

static
int
bw_flist__traverse(bw_flist* flist, visitproc visit, void* arg) {
	_s6 item;
	_s6 cpos = flist->flist_cpos;

	item = -1;
	while ((item += 1) <= cpos) {
		if (visit(*bw_flist__getref(flist, item), arg) < 0)
			return -1;
	}
	return 0;
}

static
void
bw_flist__addref_free1(bw_flist* flist, _s5 shiftlevel_stop, void** rootptr_old) {
}

static
PyObject**
bw_flist__addref(bw_flist* flist) {
	void** pptr;
	void** line;
	void** rootptr_old;
	_s6 cpos;
	_s6 precached_cpos;
	_s6 item;
	_s6 index;
	_s3 shiftlevel;
	_s3 shiftlevel_stop;
	_s3 shiftlevel_start;
	_u3 linesize_p2 = flist->flist_linesize_p2;
	
again:
	pptr = (void**)&flist->flist_rootptr;
	precached_cpos = flist->flist_precached_cpos;
	shiftlevel = flist->flist_shiftlevel;
	if ((cpos = flist->flist_cpos) < precached_cpos) {
		cpos = flist->flist_cpos + 1;
		item = cpos;
		while (shiftlevel > 0) {
			shiftlevel -= linesize_p2;
			index = item>>shiftlevel;
			// zeroed top index bits.
			item ^= index<<shiftlevel;
			pptr = &((void**)*pptr)[index];
		}
		flist->flist_cpos = cpos;
		return (PyObject**)pptr;
	}

	precached_cpos += (((_s6)1)<<linesize_p2);
	item = flist->flist_cpos + 1;
	if (precached_cpos >= (((_s6)1)<<shiftlevel)) {
		// new root line on top
		if ((line = PyMem_MALLOC(sizeof(void*)<<linesize_p2)) == NULL)
			goto failed;
		shiftlevel += linesize_p2;
		flist->flist_shiftlevel = shiftlevel;
		rootptr_old = flist->flist_rootptr;
		line[0] = flist->flist_rootptr;
		flist->flist_rootptr = line;
		pptr = (void**)&flist->flist_rootptr;
		fprintf(stderr, "2.0\n");
		while ((shiftlevel -= linesize_p2) > 0) {
			fprintf(stderr, "2.1\n");
			index = item>>shiftlevel;
			// zeroed high index bits.
			item ^= index<<shiftlevel;
			if ((line = PyMem_MALLOC(
					sizeof(void*)<<linesize_p2)) == NULL) {
				shiftlevel_stop = shiftlevel;
				shiftlevel = flist->flist_shiftlevel;
				pptr = (void**)&flist->flist_rootptr;
				item = flist->flist_cpos + 1;
				while ((shiftlevel -= linesize_p2) > shiftlevel_stop) {
					index = item>>shiftlevel;
					// zeroed top index bits.
					item ^= index<<shiftlevel;
					line = ((void**)*pptr)[index];
			fprintf(stderr, "2.1 ((void**(%p))*pptr(%p))[index(%lld)] = line(%p)\n", *pptr, pptr, index, line);
					pptr = &((void**)*pptr)[index];
					PyMem_FREE(line);
				}
				flist->flist_rootptr = rootptr_old;
				flist->flist_shiftlevel -= linesize_p2;
				goto failed;
			}
			fprintf(stderr, "2.2 ((void**(%p))*pptr(%p))[index(%lld)] = line(%p)\n", *pptr, pptr, index, line);
			((void**)*pptr)[index] = line;
			pptr = &((void**)*pptr)[index];
		}
	} else {
		_s6 previtem;
		_s6 previndex;

		previtem = flist->flist_cpos;
		shiftlevel_start = -1;
		fprintf(stderr, "3.0\n");
		while ((shiftlevel -= linesize_p2) > 0) {
			index = item>>shiftlevel;
			// zeroed top index bits.
			item ^= index<<shiftlevel;
			if (shiftlevel_start < 0) {
				if (previtem < 0)
					previndex = -1;
				else {
					previndex = previtem>>shiftlevel;
					// zeroed top index bits.
					previtem ^= previndex<<shiftlevel;
				}
				if (previndex != index)
					shiftlevel_start = shiftlevel;
			}
			fprintf(stderr, "3.1\n");
			if (shiftlevel_start > 0) {
				fprintf(stderr, "3.2\n");
				if ((line = PyMem_MALLOC(sizeof(void*)<<linesize_p2)) == NULL) {
					shiftlevel_stop = shiftlevel;
					shiftlevel = flist->flist_shiftlevel;
					pptr = (void**)&flist->flist_rootptr;
					item = flist->flist_cpos + 1;
					fprintf(stderr, "3.2.1 shiftlevel_stop=%d shiftlevel=%d\n", (int)shiftlevel_stop, (int)shiftlevel);
					while ((shiftlevel -= linesize_p2) > shiftlevel_stop) {
						index = item>>shiftlevel;
						// zeroed top index bits.
						item ^= index<<shiftlevel;
						line = ((void**)*pptr)[index];
						//fprintf(stderr, "3, item=%lld index=%lld line=%p\n", item, index, line); 
			fprintf(stderr, "3.5 ((void**(%p))*pptr(%p))[index(%lld)] = line(%p)\n", *pptr, pptr, index, line);
						pptr = &((void**)line)[index];
						if (shiftlevel <= shiftlevel_start)
							PyMem_FREE(line);
					}
					goto failed;
				}
				fprintf(stderr, "3.3 ((void**(%p))*pptr(%p))[index(%lld)] = line(%p)\n", *pptr, pptr, index, line);
				((void**)*pptr)[index] = line;
			}
			pptr = &((void**)*pptr)[index];
		}
	}
	flist->flist_precached_cpos = precached_cpos;
	goto again;
failed:
	PyErr_NoMemory();
	return NULL;
}

static
PyObject*
bw_flist__sub(bw_flist* flist) {
	PyObject* ob;
	_s6 cpos;
	_s6 item;
	_s6 index;
	_s6 nextitem;
	_s6 nextindex;
	_s6 precached_cpos;
	_s3 shiftlevel = flist->flist_shiftlevel;
	_u3 linesize_p2 = flist->flist_linesize_p2;

	cpos = flist->flist_cpos;
	precached_cpos = flist->flist_precached_cpos;
	
	if (cpos < 0) {
		PyErr_SetString(PyExc_IndexError, "flist out of range");
		return NULL;
	}
	item = flist->flist_cpos - 1;

	ob = *(PyObject**)bw_flist__getref(flist, cpos);
	if ((precached_cpos - item) < (((_s6)1)<<linesize_p2)) {
		flist->flist_cpos = item;
		return ob;
	}
	nextitem = flist->flist_cpos;
#if 0
	while ((shiftlevel -= linesize_p2) > 0) {
		index = item>>shiftlevel;
		// zeroed top index bits.
		item ^= index<<shiftlevel;
		if (shiftlevel_start < 0) {
			if (previtem < 0)
				nextindex = -1;
			else {
				nextindex = previtem>>shiftlevel;
				// zeroed top index bits.
				nextitem ^= previndex<<shiftlevel;
			}
			if (nextindex != index)
				shiftlevel_start = shiftlevel;
		}

	}
	for (;;) {
		
	}
#endif
	
}

static
PyObject*
bw_list__pop(bw_flist* flist) {
	PyObject* ob;
	
	if ((ob = bw_flist__sub(flist)) == NULL)
		goto failed;
	return ob; // borrowed refcount
failed:
	return NULL;
}

static
PyObject*
bw_flist__append(bw_flist* flist, PyObject* ob) {
	PyObject** ref;

	if ((ref = bw_flist__addref(flist)) == NULL)
		return NULL;
	Py_INCREF(ob);
	*ref = ob;
	Py_INCREF(Py_None);
	return Py_None;
}

static
PyObject*
bw_flist__extend(bw_flist* flist, PyObject* seq) {
	PyObject* iter;
	PyObject* next = NULL;
	PyObject** ref;

	if ((iter = PyObject_GetIter(seq)) == NULL)
		goto failed;

	while ((next = PyIter_Next(iter)) != NULL) {
		if ((ref = bw_flist__addref(flist)) == NULL)
			goto failed;
		*ref = next; // borrowed refcount from PyIter_Next
	}
	if (PyErr_Occurred())
		goto failed;
	Py_DECREF(iter);
	Py_INCREF(Py_None);
	return Py_None;
failed:
	Py_XDECREF(iter);
	Py_XDECREF(next);
	return NULL;
}

static
PyObject*
bw_flist__getitem(bw_flist* flist, Py_ssize_t item) {
	PyObject* ret;

	bw_flist__rdlock_CHECK(flist, NULL);
	if (item < 0 || item > flist->flist_cpos) {
		PyErr_SetString(PyExc_IndexError, "flist index out of range");
		return NULL;
	}
	ret = *bw_flist__getref(flist, item);
	Py_INCREF(ret);
	return ret;
}

static
int
bw_flist__setitem(bw_flist* flist, Py_ssize_t item, PyObject* val) {
	PyObject** ref;
	PyObject* old;

	if (val == NULL) {
		PyErr_Format(PyExc_TypeError, "'%s' object doesn't support item deletion. Use billow.ring() type instead", Py_TYPE(flist)->tp_name);
		return -1;
	}
	bw_flist__wrlock_CHECK(flist, -1);
	if (item < 0 || item > flist->flist_cpos) {
		PyErr_SetString(PyExc_IndexError, "flist index out of range");
		return -1;
	}
	ref = bw_flist__getref(flist, item);
	old = *ref;
	Py_INCREF(val);
	*ref = val;
	Py_DECREF(old);
	return 0;
}

static
Py_ssize_t
bw_flist__length(bw_flist* flist) {
	return flist->flist_cpos + 1;
}

static
PyObject*
bw_flist__repr(bw_flist* flist) {
	PyObject* ret = NULL;
	PyObject* list = NULL;
	PyObject* tuple = NULL;
	PyObject* sep = NULL;
	PyObject* s = NULL;
	_s6 cpos = flist->flist_cpos;
	_s6 item;
	_sA i;


	bw_flist__rdlock_SET(flist, NULL);
	if ((i = Py_ReprEnter((PyObject*)flist)) != 0)
		return i > 0 ? PyString_FromFormat("%s(...)", Py_TYPE(flist)->tp_name) : NULL;
	if ((sep = PyString_FromString(", ")) == NULL)
		goto failed;
	if ((tuple = PyTuple_New(12)) == NULL)
		goto failed;
	if ((list = PyList_New(0)) == NULL)
		goto failed;
	item = -1;
	while ((item += 1) <= cpos)
		PyList_Append(list, *bw_flist__getref(flist, item));
	if ((s = PyString_FromString(Py_TYPE(flist)->tp_name)) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 0, s);
	if ((s = PyString_FromString("(")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 1, s);
	if ((s = PyObject_Repr(list)) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 2, s);
	if ((s = PyString_FromString(", linesize=")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 3, s);
	if ((s = PyString_FromFormat("%d",
			(int)(((int)1)<<flist->flist_linesize_p2))) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 4, s);
	if ((s = PyString_FromString(", frozenlock=")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 5, s);
	if ((s = PyString_FromFormat("%d", (int)flist->flist_frozenlock)) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 6, s);
	if ((s = PyString_FromString(")")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 7, s);
	if ((sep = PyString_FromString("")) == NULL)
		goto failed;
	ret = _PyString_Join(sep, tuple);
failed:
	Py_ReprLeave((PyObject*)flist);
	bw_flist__rdlock_UNSET(flist);
	Py_XDECREF(sep);
	Py_XDECREF(tuple);
	Py_XDECREF(list);
	return ret;
}

static
int
bw_flist__getlinesize_p2(Py_ssize_t linesize) {
	int linesize_p2;

	if (linesize < PyBILLOW_FLIST_LINEMIN) {
		PyErr_Format(PyExc_ValueError, "linesize < %d (%d)", (int)PyBILLOW_FLIST_LINEMIN, (int)linesize);
		return -1;
	}
	if (linesize > PyBILLOW_FLIST_LINEMAX) {
		PyErr_Format(PyExc_ValueError, "linesize > %d (%d)", (int)PyBILLOW_FLIST_LINEMAX, (int)linesize);
		return -1;
	}
	if ((linesize&(linesize - 1)) != 0) {
		PyErr_Format(PyExc_ValueError, "linesize must be power of 2 (%d)", (int)linesize);
		return -1;
	}
	for (linesize_p2 = 0; (1<<linesize_p2) < linesize; linesize_p2++);
	return linesize_p2;
}

static
bw_flist*
bw_flist__new(PyTypeObject* tp, PyObject* args, PyObject* kwargs) {
	char* kwlist[] = {
		"sequence",
		"linesize",
		"maxlimit",
		"differ",
		"frozenlock",
		NULL
	};
	bw_flist* flist = NULL;
	PyObject* seq = NULL;
	_s5 frozenlock = 0;
	_s5 differ = -1;
	_s5 maxlimit = 1073741824;
	_s5 linesize = PyBILLOW_FLIST_LINEDEFAULT;
	_u3 linesize_p2;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs,
			"|Oiiii:__new__", kwlist,
			&seq, &linesize, &maxlimit, &differ, &frozenlock))
		return NULL;

	if ((flist = (bw_flist*)tp->tp_alloc(tp, 0)) == NULL)
		goto failed;
	if ((linesize_p2 = bw_flist__getlinesize_p2(linesize)) < 0)
		goto failed;
	//if (differ = bw_flist_getdiffer(differ, linesize_p2);
	//maxlimit = bw_flist_getmaxlimit(maxlimit);

	// Нет ни одного листа ни прямой, ни косвенной адресации, но для
	// роста соответствует последовательному приращению порциями по
	//
	flist->flist_cpos = -1;
	flist->flist_precached_cpos = -1;
	flist->flist_locked = 0;
	flist->flist_shiftlevel = 0;
	flist->flist_linesize_p2 = linesize_p2;
	flist->flist_frozenlock = !!frozenlock;
	if (seq != NULL) {
		PyObject* iter;
		PyObject* next = NULL;
		PyObject** ref;

		if ((iter = PyObject_GetIter(seq)) == NULL)
			goto failed;

		while ((next = PyIter_Next(iter)) != NULL) {
			if ((ref = bw_flist__addref(flist)) == NULL) {
				Py_DECREF(iter);
				goto failed;
			}
			*ref = next; // borrowed refcount from PyIter_Next
		}
		if (PyErr_Occurred())
			goto failed;
		Py_DECREF(iter);
	}
	return flist;
failed:
	Py_XDECREF(flist);
	return NULL;
}

static PySequenceMethods bw_flist__sequence = {
	.sq_length = (lenfunc)bw_flist__length,
	.sq_item = (ssizeargfunc)bw_flist__getitem,
	.sq_ass_item = (ssizeobjargproc)bw_flist__setitem,
	//(objobjproc)bw_flist__contains,	/* sq_contains */
};

static PyMappingMethods bw_flist__mapping = {
	.mp_length = (lenfunc)bw_flist__length,
//	.mp_subscript = (binaryfunc)bw_flist__subscript,
};

static PyMethodDef bw_flist__methods[] = {
//{ "truncate", (PyCFunction)bw_flist_truncate, METH_O, ""/*doc_truncate*/ },
//{ "pop", (PyCFunction)bw_flist_pop, METH_NOARGS, ""/*doc_pop*/ },
{ "push", (PyCFunction)bw_flist__append, METH_O, ""/*doc_push*/ },
{ "append", (PyCFunction)bw_flist__append, METH_O, ""/*doc_push*/ },
{ "extend", (PyCFunction)bw_flist__extend, METH_O, ""/*doc_extend*/ },
//{ "replace_first", (PyCFunction)bw_flist_replace_first, METH_O, ""/*doc_replace_first*/ },
//{ "replace_last", (PyCFunction)bw_flist_replace_last, METH_O, ""/*doc_replace_last*/ },
//{ "change_first", (PyCFunction)bw_flist_change_first, METH_O, ""/*doc_change_first*/ },
//{ "change_last", (PyCFunction)bw_flist_change_last, METH_O, ""/*doc_change_last*/ },
//{ "__reversed__", (PyCFunction)bw_flist__reversed, METH_NOARGS, ""/*doc_flist__reversed*/ },
{ NULL }
};

static PyGetSetDef bw_flist__getset[] = {
//{ "refcount", (getter)bw_flist_refcount__get, (setter)NULL, /*doc_refcount*/"", NULL },
{ NULL }
};

PyTypeObject
BillowFlist_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"billow.flist", sizeof(bw_flist), 0,
	.tp_dealloc = (destructor)bw_flist__dealloc,
	.tp_repr = (reprfunc)bw_flist__repr,
	.tp_as_sequence = &bw_flist__sequence,
	.tp_as_mapping = &bw_flist__mapping,
	.tp_getattro = PyObject_GenericGetAttr,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE,
	.tp_doc = "",//bw_flist_doc,
	.tp_traverse = (traverseproc)bw_flist__traverse,
	.tp_clear = (inquiry)bw_flist__clear,
	//.tp_iter = (getiterfunc)bw_flist__iter,
	.tp_methods = bw_flist__methods,
	.tp_getset = bw_flist__getset,
	.tp_alloc = PyType_GenericAlloc,
	.tp_new = (newfunc)bw_flist__new,
	.tp_free = PyObject_GC_Del,
};
