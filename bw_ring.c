/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#include <Python.h>

#include "px_types.h"
#include "px_genlink.h"
#include "pycompat.h"
#include "billow.h"
#include "bw_ring.h"
#include "bw_debug.h"

static
void
bw_ring__default(bw_ring_t* ring) {
	const bool order = 0;
	ring->ring_head.hd_n.n_p.link[!!order] = &ring->ring_stop.hd_n.n_p;
	ring->ring_head.hd_n.n_p.link[+!order] = &ring->ring_stop.hd_n.n_p;
	ring->ring_head.hd_n.n_body = NULL;
	ring->ring_stop.hd_n.n_p.link[!!order] = &ring->ring_head.hd_n.n_p;
	ring->ring_stop.hd_n.n_p.link[+!order] = &ring->ring_head.hd_n.n_p;
	ring->ring_stop.hd_n.n_body = NULL;
	ring->ring_stop.hd_state = NULL;
}

static
int
bw_ring__tp_clear(bw_ring_t* ring) {
	px_genlink_t* n = &ring->ring_head.hd_n.n_p;
	px_genlink_t* stop = &ring->ring_head.hd_n.n_p;
	px_genlink_t* p;
	bw_ring_state_t *state = ring->ring_head.hd_state;
	bw_ring_node_t* node;
	PyObject* item;
	bool order = !!ring->ring_order;

	if ((state->state_cursors -= 1) > 0) {
		PxQCL_selfremove(&ring->ring_stop.hd_n.n_p, order);
		PxQCL_selfremove(&ring->ring_head.hd_n.n_p, order);
		return 0;
	}
	for (n = n->link[order];;) {
		p = n;
		n = p->link[order];
		if (p == stop)
			break;
		node = px_c_cast(bw_ring_node_t, n_p, p);
		if ((item = node->n_body) == NULL) {
			continue;
		}
		state->state_elements -= 1;
		PxQCL_selfremove(p, order);
		Py_DECREF(item);
		PyMem_FREE(node);
	}
	PxQCL_selfremove(&ring->ring_stop.hd_n.n_p, order);
	PxQCL_selfremove(&ring->ring_head.hd_n.n_p, order);
	Py_DECREF(state->state_anchor);
	PyMem_FREE(ring->ring_head.hd_state);
	return 0;
}

static
void
bw_ring__tp_dealloc(bw_ring_t* ring) {
	PyObject_GC_UnTrack((PyObject*)ring);
#if PY_VERSION_HEX < 0x03080000
	Py_TRASHCAN_SAFE_BEGIN(ring)
#else /* PY_VERSION_HEX < 0x03080000 */
	Py_TRASHCAN_BEGIN(ring, bw_ring__tp_dealloc)
#endif /* PY_VERSION_HEX < 0x03080000 */
	bw_ring__tp_clear(ring);
	Py_TYPE(ring)->tp_free(ring);
#if PY_VERSION_HEX < 0x03080000
	Py_TRASHCAN_SAFE_END(ring)
#else /* PY_VERSION_HEX < 0x03080000 */
    	Py_TRASHCAN_END
#endif /* PY_VERSION_HEX < 0x03080000 */
}

static
int
bw_ring__tp_traverse(bw_ring_t* ring, visitproc visit, void* arg) {
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	px_genlink_t* stop = p;
	bw_ring_node_t* node;
	PyObject* item;
	int ret = 0;
	bool order = !!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop)
			break;
		node = px_c_cast(bw_ring_node_t, n_p, p);
		if ((item = node->n_body) == NULL) {
			bw_ring_head_t* head = px_c_cast(bw_ring_head_t, hd_n, node);
			bw_ring_t* sibling;
			if (head->hd_state == NULL)
				continue;
			sibling = px_c_cast(bw_ring_t, ring_head, head);
			if ((ret = visit((PyObject*)sibling, arg)) < 0)
				goto result;
			continue;
		}
		if ((ret = visit(item, arg)) < 0)
			goto result;
	}
	ret = visit(ring->ring_head.hd_state->state_anchor, arg);
result:
	return ret;
}

static
PyObject*
bw_ring__tp_iternext(bw_ring_t* ring) {
	px_genlink_t* n = &ring->ring_head.hd_n.n_p;
	px_genlink_t* p = n;
	px_genlink_t* stop = &ring->ring_stop.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	int order = ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop)
			return NULL;
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((ret = node->n_body) != NULL) // data
			break;
	}
	PxQCL_selfremove(p, order);
	PxQCL_selfinsert_before(p, n, order);

	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_ring__tp_new(PyTypeObject* tp, PyObject* args, PyObject* kwargs) {
	static char* kwlist[] = {
		"sequence",
		"anchor",
		"order",
		NULL
	};
	bw_ring_t* ring;
	bw_ring_state_t* state;
	PyObject* seq = NULL;
	PyObject* anchor = Py_None;
	int order = 0;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOi:ring", kwlist,
				&seq, &anchor, &order))
		return NULL;
	order = !!order;
	if ((ring = (bw_ring_t*)tp->tp_alloc(tp, 0)) == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	bw_ring__default(ring);
	ring->ring_order = order;

	if ((ring->ring_head.hd_state = state =
				PyMem_MALLOC(sizeof(*state))) == NULL)
		goto failed;
	state->state_elements = 0;
	state->state_cursors = 1;
	Py_INCREF(anchor);
	state->state_anchor = anchor;

	if (seq != NULL) {
		px_genlink_t* p = &ring->ring_stop.hd_n.n_p;
		bw_ring_node_t* node;
		PyObject* iter;
		PyObject* next;

		if ((iter = PyObject_GetIter(seq)) == NULL)
			goto failed;
		for (;;) {
			BW_YIELD();
			if ((next = PyIter_Next(iter)) == NULL) {
				if (PyErr_Occurred()) {
					Py_DECREF(iter);
					goto failed;
				}
				Py_DECREF(iter);
				break;
			}
			if ((node = PyMem_MALLOC(sizeof(*node))) == NULL) {
				Py_DECREF(next);
				Py_DECREF(iter);
				PyErr_NoMemory();
				goto failed;
			}
			node->n_body = next;
			PxQCL_selfinsert_after(&node->n_p, p, +!order);
			ring->ring_head.hd_state->state_elements += 1;
		}
	}
	//PyObject_GC_Track(ring);
	return (PyObject*)ring;
failed:
	Py_DECREF(ring);
	return NULL;
}

static
PyObject*
bw_ring__tp_repr(bw_ring_t* ring) {
	bw_ring_state_t* state = ring->ring_head.hd_state;
	PyObject* repr = NULL;
	PyObject* list = NULL;
	PyObject* tuple = NULL;
	PyObject* sep = NULL;
	PyObject* item;
	PyObject* s;
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	px_genlink_t* stop = &ring->ring_stop.hd_n.n_p;
	bw_ring_node_t* node;
	_sA i;
	bool order = !!ring->ring_order;

	if ((i = Py_ReprEnter((PyObject*)ring)) != 0) {
		return i > 0 ? PyString_FromFormat("%s(...)", Py_TYPE(ring)->tp_name) : NULL;
    }
	if ((tuple = PyTuple_New(8)) == NULL)
		goto failed;
	if ((list = PyList_New(0)) == NULL)
		goto failed;

	for (;;) {
		BW_YIELD();
		if ((p = p->link[order]) == stop)
			break;
		node = px_c_cast(bw_ring_node_t, n_p, p);
		if ((item = node->n_body) == NULL)
			continue;
		if (PyList_Append(list, item))
			goto failed;
	}
	if ((s = PyString_FromString(Py_TYPE(ring)->tp_name)) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 0, s);
	if ((s = PyString_FromString("(")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 1, s);
	if ((s = PyObject_Repr(list)) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 2, s);
	if ((s = PyString_FromString(", order=")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 3, s);
	if ((s = PyString_FromString(order ? "1" : "0")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 4, s);
	if ((s = PyString_FromString(", anchor=")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 5, s);
	if ((s = PyObject_Repr(state->state_anchor)) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 6, s);
	if ((s = PyString_FromString(")")) == NULL)
		goto failed;
	PyTuple_SET_ITEM(tuple, 7, s);

	if ((sep = PyString_FromString("")) == NULL)
		goto failed;
	repr = _PyString_Join(sep, tuple);
failed:
	Py_ReprLeave((PyObject*)ring);
	Py_XDECREF(sep);
	Py_XDECREF(tuple);
	Py_XDECREF(list);
	return repr;
}

#if 0
static
PyObject*
bw_ring__tp_richcompare(bw_ring_t* ring, bw_ring_t* w, int op) {
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	px_genlink_t* stop = &ring->ring_stop.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	PyObject* item;
	PyObject* item2;
	bool order = !!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			item = Py_None;
			break;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((item = node->n_body) == NULL)
			continue;
		break;
	}
	if (!PyObject_TypeCheck(w, &BillowRing_Type)) {
		Py_INCREF(item);
		Py_INCREF(w);
		ret =  PyObject_RichCompare(item, (PyObject*)w, op);
		Py_DECREF(item);
		Py_DECREF(w);
		return ret;
	}


	p = &w->ring_head.hd_n.n_p;
	stop = &w->ring_stop.hd_n.n_p;
	order = !!w->ring_order;
	for (;;) {
		if ((p = p->link[order]) == stop) {
			item = Py_None;
			break;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((item2 = node->n_body) == NULL)
			continue;
		break;
	}
	Py_INCREF(item);
	Py_INCREF(item);
	ret = PyObject_RichCompare(item, item2, op);
	Py_DECREF(item);
	Py_DECREF(item2);
	return ret;
}
#else
static
PyObject*
bw_ring__tp_richcompare(bw_ring_t* ring, bw_ring_t* w, int op) {
	if (!PyObject_TypeCheck(w, &bw_ring_Type))
		return PyObject_RichCompare(
				ring->ring_head.hd_state->state_anchor,
				(PyObject*)w, op);
	return PyObject_RichCompare(
			ring->ring_head.hd_state->state_anchor,
			   w->ring_head.hd_state->state_anchor, op);
	

}
#endif

static
Py_ssize_t
bw_ring__mp_length(bw_ring_t* ring) {
	return (Py_ssize_t)ring->ring_head.hd_state->state_elements;
}

static
PyObject*
bw_ring__roll_by0(bw_ring_t* ring) {
	PyObject* ret = bw_ring__tp_iternext(ring);
	if (ret == NULL)
		PyErr_SetNone(PyExc_StopIteration);
	return ret;
}

static
PyObject*
bw_ring__roll_by1(bw_ring_t* ring) {
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	px_genlink_t* n = p;
	px_genlink_t* stop = &ring->ring_stop.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	bool order = +!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			PyErr_SetNone(PyExc_StopIteration);
			return NULL;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((ret = node->n_body) != NULL)
			break;
	}
	PxQCL_selfremove(p, order);
	PxQCL_selfinsert_before(p, n, order);

	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_ring__rolltail_by0(bw_ring_t* ring) {
	px_genlink_t* p = &ring->ring_stop.hd_n.n_p;
	px_genlink_t* n = p;
	px_genlink_t* stop = &ring->ring_head.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	bool order = +!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			PyErr_SetNone(PyExc_StopIteration);
			return NULL;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((ret = node->n_body) != NULL)
			break;
	}
	PxQCL_selfremove(p, order);
	PxQCL_selfinsert_before(p, n, order);

	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_ring__rolltail_by1(bw_ring_t* ring) {
	px_genlink_t* p = &ring->ring_stop.hd_n.n_p;
	px_genlink_t* n = p;
	px_genlink_t* stop = &ring->ring_head.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	bool order = !!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			PyErr_SetNone(PyExc_StopIteration);
			return NULL;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((ret = node->n_body) != NULL)
			break;
	}
	PxQCL_selfremove(p, order);
	PxQCL_selfinsert_before(p, n, order);

	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_ring__get_by0(bw_ring_t* ring, PyObject* ob) {
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	px_genlink_t* stop = &ring->ring_stop.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	bool order = !!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			Py_INCREF(ob);
			return ob;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((ret = node->n_body) != NULL)
			break;
	}
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_ring__get_by1(bw_ring_t* ring, PyObject* ob) {
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	px_genlink_t* stop = &ring->ring_stop.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	bool order = +!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			Py_INCREF(ob);
			return ob;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((ret = node->n_body) != NULL)
			break;
	}
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_ring__gettail_by0(bw_ring_t* ring, PyObject* ob) {
	px_genlink_t* p = &ring->ring_stop.hd_n.n_p;
	px_genlink_t* stop = &ring->ring_head.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	bool order = +!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			Py_INCREF(ob);
			return ob;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((ret = node->n_body) != NULL)
			break;
	}
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_ring__gettail_by1(bw_ring_t* ring, PyObject* ob) {
	px_genlink_t* p = &ring->ring_stop.hd_n.n_p;
	px_genlink_t* stop = &ring->ring_head.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	bool order = !!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			Py_INCREF(ob);
			return ob;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((ret = node->n_body) != NULL)
			break;
	}
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_ring__pop_by0(bw_ring_t* ring) {
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	px_genlink_t* stop = &ring->ring_stop.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	bool order = !!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			PyErr_SetNone(PyExc_StopIteration);
			return NULL;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((ret = node->n_body) != NULL)
			break;
	}
	PxQCL_selfremove(p, order);
	PyMem_FREE(node);
	ring->ring_head.hd_state->state_elements -= 1;
	return ret; // borrowed refcount
}

static
PyObject*
bw_ring__pop_by1(bw_ring_t* ring) {
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	px_genlink_t* stop = &ring->ring_stop.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	bool order = +!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			PyErr_SetNone(PyExc_StopIteration);
			return NULL;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((ret = node->n_body) != NULL)
			break;
	}
	PxQCL_selfremove(p, order);
	PyMem_FREE(node);
	ring->ring_head.hd_state->state_elements -= 1;
	return ret; // borrowed refcount
}

static
PyObject*
bw_ring__poptail_by0(bw_ring_t* ring) {
	px_genlink_t* p = &ring->ring_stop.hd_n.n_p;
	px_genlink_t* stop = &ring->ring_head.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	bool order = +!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			PyErr_SetNone(PyExc_StopIteration);
			return NULL;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((ret = node->n_body) != NULL)
			break;
	}
	PxQCL_selfremove(p, order);
	PyMem_FREE(node);
	ring->ring_head.hd_state->state_elements -= 1;
	return ret; // borrowed refcount
}

static
PyObject*
bw_ring__poptail_by1(bw_ring_t* ring) {
	px_genlink_t* p = &ring->ring_stop.hd_n.n_p;
	px_genlink_t* stop = &ring->ring_head.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* ret;
	bool order = !!ring->ring_order;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			PyErr_SetNone(PyExc_StopIteration);
			return NULL;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((ret = node->n_body) != NULL)
			break;
	}
	PxQCL_selfremove(p, order);
	PyMem_FREE(node);
	ring->ring_head.hd_state->state_elements -= 1;
	return ret; // borrowed refcount
}

static
PyObject*
bw_ring__push_by0(bw_ring_t* ring, PyObject* ob) {
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	bw_ring_node_t* node;
	bool order = !!ring->ring_order;

	if ((node = PyMem_MALLOC(sizeof(*node))) == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(ob);
	node->n_body = ob;
	PxQCL_selfinsert_after(&node->n_p, p, order);
	ring->ring_head.hd_state->state_elements += 1;

	Py_INCREF(ring);
	return (PyObject*)ring;
}

static
PyObject*
bw_ring__push_by1(bw_ring_t* ring, PyObject* ob) {
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	bw_ring_node_t* node;
	bool order = +!ring->ring_order;

	if ((node = PyMem_MALLOC(sizeof(*node))) == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(ob);
	node->n_body = ob;
	PxQCL_selfinsert_after(&node->n_p, p, order);
	ring->ring_head.hd_state->state_elements += 1;

	Py_INCREF(ring);
	return (PyObject*)ring;
}

static
PyObject*
bw_ring__pushtail_by0(bw_ring_t* ring, PyObject* ob) {
	px_genlink_t* p = &ring->ring_stop.hd_n.n_p;
	bw_ring_node_t* node;
	bool order = +!ring->ring_order;

	if ((node = PyMem_MALLOC(sizeof(*node))) == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(ob);
	node->n_body = ob;
	PxQCL_selfinsert_after(&node->n_p, p, order);
	ring->ring_head.hd_state->state_elements += 1;

	Py_INCREF(ring);
	return (PyObject*)ring;
}

static
PyObject*
bw_ring__pushtail_by1(bw_ring_t* ring, PyObject* ob) {
	px_genlink_t* p = &ring->ring_stop.hd_n.n_p;
	bw_ring_node_t* node;
	bool order = !!ring->ring_order;

	if ((node = PyMem_MALLOC(sizeof(*node))) == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	Py_INCREF(ob);
	node->n_body = ob;
	PxQCL_selfinsert_before(&node->n_p, p, order);
	ring->ring_head.hd_state->state_elements += 1;

	Py_INCREF(ring);
	return (PyObject*)ring;
}

static
PyObject*
bw_ring__stackextend_by0(bw_ring_t* ring, PyObject* seq) {
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* next = NULL;
	PyObject* iter;
	bool order = !!ring->ring_order;

	if ((iter = PyObject_GetIter(seq)) == NULL)
		return NULL;
	for (;;) {
		BW_YIELD();
		if ((next = PyIter_Next(iter)) == NULL) {
			if (PyErr_Occurred()) {
				Py_DECREF(iter);
				goto failed;
			}
			break;
		}
		if ((node = PyMem_MALLOC(sizeof(*node))) == NULL) {
			Py_DECREF(next);
			PyErr_NoMemory();
			goto failed;
		}
		node->n_body = next;
		PxQCL_selfinsert_after(&node->n_p, p, order);
		ring->ring_head.hd_state->state_elements += 1;
	}
	Py_DECREF(iter);

	Py_INCREF(ring);
	return (PyObject*)ring;
failed:
	Py_DECREF(iter);
	return NULL;
}

static
PyObject*
bw_ring__stackextend_by1(bw_ring_t* ring, PyObject* seq) {
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* next = NULL;
	PyObject* iter;
	bool order = +!ring->ring_order;

	if ((iter = PyObject_GetIter(seq)) == NULL)
		return NULL;
	for (;;) {
		BW_YIELD();
		if ((next = PyIter_Next(iter)) == NULL) {
			if (PyErr_Occurred()) {
				Py_DECREF(iter);
				goto failed;
			}
			break;
		}
		if ((node = PyMem_MALLOC(sizeof(*node))) == NULL) {
			Py_DECREF(next);
			PyErr_NoMemory();
			goto failed;
		}
		node->n_body = next;
		PxQCL_selfinsert_after(&node->n_p, p, order);
		ring->ring_head.hd_state->state_elements += 1;
	}
	Py_DECREF(iter);

	Py_INCREF(ring);
	return (PyObject*)ring;
failed:
	Py_DECREF(iter);
	return NULL;
}

static
PyObject*
bw_ring__extend_by0(bw_ring_t* ring, PyObject* seq) {
	px_genlink_t* p = &ring->ring_stop.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* next = NULL;
	PyObject* iter;
	bool order = +!ring->ring_order;

	if ((iter = PyObject_GetIter(seq)) == NULL)
		return NULL;
	for (;;) {
		BW_YIELD();
		if ((next = PyIter_Next(iter)) == NULL) {
			if (PyErr_Occurred()) {
				Py_DECREF(iter);
				goto failed;
			}
			break;
		}
		if ((node = PyMem_MALLOC(sizeof(*node))) == NULL) {
			Py_DECREF(next);
			PyErr_NoMemory();
			goto failed;
		}
		node->n_body = next;
		PxQCL_selfinsert_after(&node->n_p, p, order);
		ring->ring_head.hd_state->state_elements += 1;
	}
	Py_DECREF(iter);

	Py_INCREF(ring);
	return (PyObject*)ring;
failed:
	Py_DECREF(iter);
	return NULL;
}

static
PyObject*
bw_ring__extend_by1(bw_ring_t* ring, PyObject* seq) {
	px_genlink_t* p = &ring->ring_stop.hd_n.n_p;
	bw_ring_node_t* node;
	PyObject* next = NULL;
	PyObject* iter;
	bool order = !!ring->ring_order;

	if ((iter = PyObject_GetIter(seq)) == NULL)
		return NULL;
	for (;;) {
		BW_YIELD();
		if ((next = PyIter_Next(iter)) == NULL) {
			if (PyErr_Occurred()) {
				Py_DECREF(iter);
				goto failed;
			}
			break;
		}
		if ((node = PyMem_MALLOC(sizeof(*node))) == NULL) {
			Py_DECREF(next);
			PyErr_NoMemory();
			goto failed;
		}
		node->n_body = next;
		PxQCL_selfinsert_after(&node->n_p, p, order);
		ring->ring_head.hd_state->state_elements += 1;
	}
	Py_DECREF(iter);

	Py_INCREF(ring);
	return (PyObject*)ring;
failed:
	Py_DECREF(iter);
	return NULL;
}

static
PyObject*
bw_ring__restart(bw_ring_t* ring) {
	bool order = !!ring->ring_order;
	PxQCL_selfremove(&ring->ring_head.hd_n.n_p, order);
	PxQCL_selfinsert_after(&ring->ring_head.hd_n.n_p,
			       &ring->ring_stop.hd_n.n_p, order);
	Py_INCREF(ring);
	return (PyObject*)ring;
}

static
PyObject*
bw_ring__one(bw_ring_t* ring) {
	bw_ring_state_t* state = ring->ring_head.hd_state;

	if (state != NULL && state->state_cursors > 1) {
		// too many cursors
		PyErr_SetNone(BwExc_ExpectOneError);
		return NULL;
	}
	Py_INCREF(ring);
	return (PyObject*)ring;
}

static
PyObject*
bw_ring__siblings(bw_ring_t* ring) {
	bw_ring_state_t* state = ring->ring_head.hd_state;
	px_genlink_t* p = &ring->ring_head.hd_n.n_p;
	px_genlink_t* stop = p;
	bw_ring_node_t* node;
	bw_ring_head_t* head;
	bw_ring_t* ring2;
	PyObject* list;
	PyObject* item;
	bool order = !!ring->ring_order;

	if (state->state_elements > 0) {
		PyErr_SetString(PyExc_ValueError,
			"ring must be empty as container only pseudo-nodes");
		return NULL;
	}
	if ((list = PyList_New(0)) == NULL)
		return NULL;

	for (;;) {
		if ((p = p->link[order]) == stop) {
			item = Py_None;
			break;
		}
		node =  px_c_cast(bw_ring_node_t, n_p, p);
		if ((item = node->n_body) != NULL) {
			PyErr_SetNone(BwExc_ConsistentError);
			return NULL;
		}
		head = px_c_cast(bw_ring_head_t, hd_n, node);
		if (head->hd_state == NULL)
			continue;
		ring2 = px_c_cast(bw_ring_t, ring_head, head);
		if (PyList_Append(list, (PyObject*)ring2) < 0) {
			Py_DECREF(list);
			return NULL;
		}
	}
	return list;
}

static
PyObject*
bw_ring__elements__get(bw_ring_t* ring, void* args) {
	bw_ring_state_t* state = ring->ring_head.hd_state;
	return PyInt_FromLong(state->state_elements);
}

static
PyObject*
bw_ring__cursors__get(bw_ring_t* ring, void* args) {
	bw_ring_state_t* state = ring->ring_head.hd_state;
	return PyInt_FromLong(state->state_cursors);
}

static
PyObject*
bw_ring__anchor__get(bw_ring_t* ring, void* args) {
	PyObject* anchor = ring->ring_head.hd_state->state_anchor;
	Py_INCREF(anchor);
	return anchor;
}

static
int
bw_ring__anchor__set(bw_ring_t* ring, PyObject* anchor, void* args) {
	bw_ring_state_t* state = ring->ring_head.hd_state;

	Py_DECREF(state->state_anchor);
	Py_INCREF(anchor);
	state->state_anchor = anchor;
	return 0;
}

static
PyObject*
bw_ring__order__get(bw_ring_t* ring, void* args) {
	return PyInt_FromLong(ring->ring_order);
}

static
int
bw_ring__order__set(bw_ring_t* ring, PyObject* val, void* args) {
	if (!PyInt_Check(val)) {
		PyErr_SetString(PyExc_TypeError, "required an int()");
		return -1;
	}
	ring->ring_order = (PyInt_AsLong(val) != 0);
	return 0;
}

static
PyObject*
bw_ring__iter(bw_ring_t* ring) {
	bw_ring_t* newring;
	bool order = !!ring->ring_order;

	if ((newring = (bw_ring_t*)(Py_TYPE(ring)->tp_alloc(Py_TYPE(ring), 0))) == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	bw_ring__default(newring);
	newring->ring_order = order;
	newring->ring_head.hd_state = ring->ring_head.hd_state;
	newring->ring_head.hd_state->state_cursors += 1;
	PxQCL_selfinsert_after(&newring->ring_head.hd_n.n_p,
				  &ring->ring_head.hd_n.n_p, order);
	PxQCL_selfinsert_before(&newring->ring_stop.hd_n.n_p,
				   &ring->ring_stop.hd_n.n_p, order);
	//PyObject_GC_Track(newring);
	return (PyObject*)newring;
}

static
PyObject*
bw_ring__reversed(bw_ring_t* ring) {
	bw_ring_t* newring;
	bool order = +!ring->ring_order;

	if ((newring = (bw_ring_t*)(Py_TYPE(ring)->tp_alloc(Py_TYPE(ring), 0))) == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	bw_ring__default(newring);
	newring->ring_order = order;
	newring->ring_head.hd_state = ring->ring_head.hd_state;
	newring->ring_head.hd_state->state_cursors += 1;
	PxQCL_selfinsert_before(&newring->ring_head.hd_n.n_p,
				   &ring->ring_stop.hd_n.n_p, order);
	PxQCL_selfinsert_after(&newring->ring_stop.hd_n.n_p,
				   &ring->ring_head.hd_n.n_p, order);
	//PyObject_GC_Track(newring);
	return (PyObject*)newring;
}

static
PyObject*
bw_ring__shadow(bw_ring_t* ring) {
	bw_ring_t* newring;
	bool order = !!ring->ring_order;

	if ((newring = (bw_ring_t*)(Py_TYPE(ring)->tp_alloc(Py_TYPE(ring), 0))) == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	bw_ring__default(newring);
	newring->ring_order = order;
	newring->ring_head.hd_state = ring->ring_head.hd_state;
	newring->ring_head.hd_state->state_cursors += 1;
	PxQCL_selfinsert_after(&newring->ring_head.hd_n.n_p,
				  &ring->ring_stop.hd_n.n_p, order);
	PxQCL_selfinsert_before(&newring->ring_stop.hd_n.n_p,
				   &ring->ring_head.hd_n.n_p, order);
	//PyObject_GC_Track(newring);
	return (PyObject*)newring;
}

static
PyObject*
bw_ring__aggrwith(bw_ring_t* ring, bw_ring_t* other) {
	bw_ring_state_t* state;
	bw_ring_state_t* other_state;
	bool order = !!ring->ring_order;
	bool other_order;

	if (Py_TYPE(ring) != Py_TYPE(other)) {
		PyErr_SetString(PyExc_TypeError, "type must be the same");
		return NULL;
	}
	if ((other_state = other->ring_head.hd_state) ==
		  (state = ring->ring_head.hd_state)) {
		PyErr_SetString(PyExc_ValueError, "objects must in differ share area state");
		return NULL;
	}
	other_order = !!other->ring_order;
	if (other_state->state_cursors == 1
	 && order == (other_order = !!other->ring_order)
	 && other->ring_head.hd_n.n_p.link[+!order] == 
	   &other->ring_stop.hd_n.n_p) {

		if (other_state->state_elements > 0) {
			px_genlink_t* n;
			px_genlink_t* p;

			state->state_elements += other_state->state_elements;
			other_state->state_elements = 0;

			p = ring->ring_stop.hd_n.n_p.link[+!order];
			p->link[!!order] = n =
				other->ring_head.hd_n.n_p.link[order];
			n->link[+!order] = p;

			ring->ring_stop.hd_n.n_p.link[+!order] = n =
				other->ring_stop.hd_n.n_p.link[+!order];
			n->link[!!order] = &ring->ring_stop.hd_n.n_p;

			other->ring_head.hd_n.n_p.link[!!order] =
				&other->ring_stop.hd_n.n_p;
			other->ring_stop.hd_n.n_p.link[+!order] =
				&other->ring_head.hd_n.n_p;

		}
	} else {
		px_genlink_t* other_stop = &other->ring_stop.hd_n.n_p;
		px_genlink_t* n = &other->ring_head.hd_n.n_p;
		px_genlink_t* p;

		for (n = n->link[other_order];;) {
			p = n;
			n = p->link[other_order];

			if (p == other_stop)
				break;
			if (px_c_cast(bw_ring_node_t, n_p, p)->n_body == NULL) 
				continue;
			state->state_elements +=1;
			other_state->state_elements -=1;
			PxQCL_selfremove(p, order);
			PxQCL_selfinsert_before(p,
				&ring->ring_stop.hd_n.n_p, order);
		}
	}

	Py_INCREF(ring);
	return (PyObject*)ring;
}

static
PyObject*
bw_ring__truncate(bw_ring_t* ring) {
	px_genlink_t* stop = &ring->ring_stop.hd_n.n_p;
	px_genlink_t* n = &ring->ring_head.hd_n.n_p;
	px_genlink_t* p;
	bw_ring_state_t* state = ring->ring_head.hd_state;
	bool order = !!ring->ring_order;

	for (n = n->link[order];;) {
		bw_ring_node_t* node;

		p = n;
		n = p->link[order];
		if (p == stop)
			break;
		node = px_c_cast(bw_ring_node_t, n_p, p);
		if (node->n_body == NULL) 
			continue;
		state->state_elements -= 1;
		PxQCL_selfremove(p, order);
		PyMem_FREE(node);
	}
	Py_INCREF(ring);
	return (PyObject*)ring;
}

static
PyObject*
bw_ring__truncate_shadow(bw_ring_t* ring) {
	px_genlink_t* stop = &ring->ring_head.hd_n.n_p;
	px_genlink_t* n = &ring->ring_stop.hd_n.n_p;
	px_genlink_t* p;
	bw_ring_state_t* state = ring->ring_head.hd_state;
	bool order = !!ring->ring_order;

	for (n = n->link[order];;) {
		bw_ring_node_t* node;

		p = n;
		n = p->link[order];
		if (p == stop)
			break;
		node = px_c_cast(bw_ring_node_t, n_p, p);
		if (node->n_body == NULL) 
			continue;
		state->state_elements -= 1;
		PxQCL_selfremove(p, order);
		PyMem_FREE(node);
	}
	Py_INCREF(ring);
	return (PyObject*)ring;
}

static
PyObject*
bw_ring__truncate_full(bw_ring_t* ring) {
	px_genlink_t* stop = &ring->ring_head.hd_n.n_p;
	px_genlink_t* n = stop;
	px_genlink_t* p;
	bw_ring_state_t* state = ring->ring_head.hd_state;
	bool order = !!ring->ring_order;

	for (n = n->link[order];;) {
		bw_ring_node_t* node;

		p = n;
		n = p->link[order];
		if (p == stop)
			break;
		node = px_c_cast(bw_ring_node_t, n_p, p);
		if (node->n_body == NULL) 
			continue;
		state->state_elements -= 1;
		PxQCL_selfremove(p, order);
		PyMem_FREE(node);
	}
	Py_INCREF(ring);
	return (PyObject*)ring;
}

static PyMethodDef bw_ring__tp_methods[] = {
{ "roll", (PyCFunction)bw_ring__roll_by0, METH_NOARGS, ""/*doc_roll_by1*/ },
{ "roll_by0", (PyCFunction)bw_ring__roll_by0, METH_NOARGS, ""/*doc_roll_by1*/ },
{ "roll_by1", (PyCFunction)bw_ring__roll_by1, METH_NOARGS, ""/*doc_roll_by1*/ },
{ "rolltail", (PyCFunction)bw_ring__rolltail_by0, METH_NOARGS, ""/*doc_rolltaill_by0*/ },
{ "rolltail_by0", (PyCFunction)bw_ring__rolltail_by0, METH_NOARGS, ""/*doc_rolltaill_by0*/ },
{ "rolltail_by1", (PyCFunction)bw_ring__rolltail_by1, METH_NOARGS, ""/*doc_rolltail_by1*/ },
{ "get", (PyCFunction)bw_ring__get_by0, METH_O, ""/*doc_get_by0*/ },
{ "get_by0", (PyCFunction)bw_ring__get_by0, METH_O, ""/*doc_get_by0*/ },
{ "get_by1", (PyCFunction)bw_ring__get_by1, METH_O, ""/*doc_pop_by1*/ },
{ "gettail", (PyCFunction)bw_ring__gettail_by0, METH_O, ""/*doc_gettail_by0*/ },
{ "gettail_by0", (PyCFunction)bw_ring__gettail_by0, METH_O, ""/*doc_gettail_by0*/ },
{ "gettail_by1", (PyCFunction)bw_ring__gettail_by1, METH_O, ""/*doc_gettail_by1*/ },
{ "pop", (PyCFunction)bw_ring__pop_by0, METH_NOARGS, ""/*doc_pop_by0*/ },
{ "pop_by0", (PyCFunction)bw_ring__pop_by0, METH_NOARGS, ""/*doc_pop_by0*/ },
{ "pop_by1", (PyCFunction)bw_ring__pop_by1, METH_NOARGS, ""/*doc_pop_by0*/ },
{ "poptail", (PyCFunction)bw_ring__poptail_by0, METH_NOARGS, ""/*doc_poptail_by0*/ },
{ "poptail_by0", (PyCFunction)bw_ring__poptail_by0, METH_NOARGS, ""/*doc_poptail_by0*/ },
{ "poptail_by1", (PyCFunction)bw_ring__poptail_by1, METH_NOARGS, ""/*doc_poptail_by1*/ },
{ "push", (PyCFunction)bw_ring__push_by0, METH_O, ""/*doc_push_by0*/ },
{ "push_by0", (PyCFunction)bw_ring__push_by0, METH_O, ""/*doc_push_by0*/ },
{ "push_by1", (PyCFunction)bw_ring__push_by1, METH_O, ""/*doc_push_by1*/ },
{ "append", (PyCFunction)bw_ring__pushtail_by0, METH_O, ""/*doc_push_by0*/ },
{ "pushtail", (PyCFunction)bw_ring__pushtail_by0, METH_O, ""/*doc_pushtail_by0*/ },
{ "pushtail_by0", (PyCFunction)bw_ring__pushtail_by0, METH_O, ""/*doc_pushtail_by0*/ },
{ "pushtail_by1", (PyCFunction)bw_ring__pushtail_by1, METH_O, ""/*doc_pushtail_by1*/ },
{ "stackextend", (PyCFunction)bw_ring__stackextend_by0, METH_O, ""/*doc_stackextend_by1*/ },
{ "stackextent_by0", (PyCFunction)bw_ring__stackextend_by0, METH_O, ""/*doc_stackextend_by0*/ },
{ "stackextend_by1", (PyCFunction)bw_ring__stackextend_by1, METH_O, ""/*doc_stackextend_by1*/ },
{ "extend", (PyCFunction)bw_ring__extend_by0, METH_O, ""/*doc_extend_by1*/ },
{ "extend_by0", (PyCFunction)bw_ring__extend_by0, METH_O, ""/*doc_extend_by0*/ },
{ "extend_by1", (PyCFunction)bw_ring__extend_by1, METH_O, ""/*doc_extend_by1*/ },
{ "one", (PyCFunction)bw_ring__one, METH_NOARGS, ""/*doc_one*/ },
{ "iter", (PyCFunction)bw_ring__iter, METH_NOARGS, ""/*doc_shadow*/ },
{ "__reversed__", (PyCFunction)bw_ring__reversed, METH_NOARGS, ""/*doc_reversed*/ },
{ "reversed", (PyCFunction)bw_ring__reversed, METH_NOARGS, ""/*doc_reversed*/ },
{ "shadow", (PyCFunction)bw_ring__shadow, METH_NOARGS, ""/*doc_shadow*/ },
{ "restart", (PyCFunction)bw_ring__restart, METH_NOARGS, ""/*doc_restart*/ },
{ "aggrwith", (PyCFunction)bw_ring__aggrwith, METH_O, ""/*doc_aggrwith*/ },
{ "truncate", (PyCFunction)bw_ring__truncate, METH_NOARGS, ""/*doc_truncate*/ },
{ "truncate_shadow", (PyCFunction)bw_ring__truncate_shadow, METH_NOARGS, ""/*doc_truncate_shadow*/ },
{ "truncate_full", (PyCFunction)bw_ring__truncate_full, METH_NOARGS, ""/*doc_truncate_full*/ },
{ "siblings", (PyCFunction)bw_ring__siblings, METH_NOARGS, ""/*doc_siblings*/ },
{ NULL }
};

static PyGetSetDef bw_ring__tp_getset[] = {
{ "elements", (getter)bw_ring__elements__get, (setter)NULL, /*doc_holds*/"", NULL },
{ "cursors", (getter)bw_ring__cursors__get, (setter)NULL, /*doc_cursors*/"", NULL },
{ "anchor", (getter)bw_ring__anchor__get, (setter)bw_ring__anchor__set, /*doc_anchor*/"", NULL },
{ "order", (getter)bw_ring__order__get, (setter)bw_ring__order__set, /*doc_order*/"", NULL },
{ NULL }
};

static PySequenceMethods bw_ring__tp_as_sequence = {
	.sq_length = (lenfunc)bw_ring__mp_length,
	//0,					/* sq_concat */
	//0,					/* sq_repeat */
	//0,					/* sq_item */
	//0,					/* sq_slice */
	//,					/* sq_ass_item */
	//0,					/* sq_ass_slice */
	//0,					/* sq_contains */
};

static PyMappingMethods bw_ring__tp_as_mapping = {
	.mp_length = (lenfunc)bw_ring__mp_length,
	0,					/* mp_subscript */
	0
};


PyTypeObject
bw_ring_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"billow.ring", sizeof(bw_ring_t), 0,
	.tp_dealloc = (destructor)bw_ring__tp_dealloc,
	.tp_repr = (reprfunc)bw_ring__tp_repr,
	.tp_as_sequence = &bw_ring__tp_as_sequence,
	.tp_as_mapping = &bw_ring__tp_as_mapping,
	.tp_getattro = PyObject_GenericGetAttr,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
	.tp_doc = "",
	.tp_traverse = (traverseproc)bw_ring__tp_traverse,
	.tp_clear = (inquiry)bw_ring__tp_clear,
	.tp_richcompare = (richcmpfunc)bw_ring__tp_richcompare,
	.tp_iter = (getiterfunc)PyObject_SelfIter,
	.tp_iternext = (iternextfunc)bw_ring__tp_iternext,
	.tp_methods = bw_ring__tp_methods,
	.tp_getset = bw_ring__tp_getset,
	.tp_alloc = PyType_GenericAlloc,
	.tp_new = (newfunc)bw_ring__tp_new,
	.tp_free = PyObject_GC_Del,
};
