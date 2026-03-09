/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#include <Python.h>

#include "pycompat.h"
#include "billow.h"
#include "bw_nil.h"
#include "bw_pair.h"
#include "bw_debug.h"

PyObject bw_NilStruct = {
	_PyObject_EXTRA_INIT
	1, &bw_nil_Type
};

static
void
bw_nil__dealloc(PyObject* self) {
}

static
int
bw_nil__null(PyObject* self) {
	return 0;
}

static
PyObject*
bw_nil__selfunary(PyObject* self) {
	//self = Py_Nil;
	Py_INCREF(self);
	return self;
}

static
PyObject*
bw_nil__divmod(PyObject* self, PyObject* arg) {
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
bw_nil__selfbinary(PyObject* self, PyObject* arg) {
	self = Py_Nil;
	Py_INCREF(self);
	return self;
}

static
PyObject*
bw_nil__selfternary(PyObject* self, PyObject* arg1, PyObject* arg2) {
	self = Py_Nil;
	Py_INCREF(self);
	return self;
}

#if PY_VERSION_HEX < 0x03000000
static
int
bw_nil__coerce(PyObject** pv, PyObject** pw) {
	Py_INCREF(*pv);
	Py_INCREF(*pw);
	return 0;
}
#endif /* PY_VERSION_HEX < 0x03000000 */

static
PyObject*
bw_nil__richcompare(PyObject* v, PyObject* w, int op) {
	PyObject* ret;
	if (v == w) {
		switch (op) {
		case Py_LT:
			ret = Py_False;
			break;
		case Py_LE:
			ret = Py_True;
			break;
		case Py_EQ:
			ret = Py_True;
			break;
		case Py_NE:
			ret = Py_False;
			break;
		case Py_GT:
			ret = Py_False;
			break;
		case Py_GE:
			ret = Py_True;
			break;
		default:
			return NULL;
		}
	} else {
		ret = Py_False;
	}
	Py_INCREF(ret);
	return ret;
}

static
PyObject*
bw_nil__repr(PyObject* self) {
	return PyString_InternFromString("Nil");
}

static
PyNumberMethods
bw_nil__number = {
	.nb_add = (binaryfunc)bw_nil__selfbinary,
	.nb_subtract = (binaryfunc)bw_nil__selfbinary,
	.nb_multiply = (binaryfunc)bw_nil__selfbinary,
#if PY_VERSION_HEX < 0x03000000
	.nb_divide = (binaryfunc)bw_nil__selfbinary,
#endif /* PY_VERSION_HEX < 0x03000000 */
	.nb_remainder = (binaryfunc)bw_nil__selfbinary,
	.nb_divmod = (binaryfunc)bw_nil__divmod,
	.nb_power = (ternaryfunc)bw_nil__selfternary,
	.nb_negative = (unaryfunc)bw_nil__selfunary,
	.nb_positive = (unaryfunc)bw_nil__selfunary,
	.nb_absolute = (unaryfunc)bw_nil__selfunary,
#if PY_VERSION_HEX < 0x03000000
	.nb_nonzero = (inquiry)bw_nil__null,
#else /* VERSION_HEX < 0x03000000 */
	.nb_bool = (inquiry)bw_nil__null,
#endif /* PY_VERSION_HEX < 0x03000000 */
	.nb_invert = (unaryfunc)bw_nil__selfunary,
	.nb_lshift = (binaryfunc)bw_nil__selfbinary,
	.nb_rshift = (binaryfunc)bw_nil__selfbinary,
	.nb_and = (binaryfunc)bw_nil__selfbinary,
	.nb_xor = (binaryfunc)bw_nil__selfbinary,
	.nb_or = (binaryfunc)bw_nil__selfbinary,
#if PY_VERSION_HEX < 0x03000000
	.nb_coerce = (coercion)bw_nil__coerce,
#endif /* PY_VERSION_HEX < 0x03000000 */
	.nb_int = (unaryfunc)bw_nil__selfunary,
#if PY_VERSION_HEX < 0x03000000
	.nb_long = (unaryfunc)bw_nil__selfunary,
#endif /* PY_VERSION_HEX < 0x03000000 */
	.nb_float = (unaryfunc)bw_nil__selfunary,
#if PY_VERSION_HEX < 0x03000000
	.nb_oct = (unaryfunc)bw_nil__selfunary,
	.nb_hex = (unaryfunc)bw_nil__selfunary,
#endif /* PY_VERSION_HEX < 0x03000000 */

	.nb_inplace_add = (binaryfunc)bw_nil__selfbinary,
	.nb_inplace_subtract = (binaryfunc)bw_nil__selfbinary,
	.nb_inplace_multiply = (binaryfunc)bw_nil__selfbinary,
#if PY_VERSION_HEX < 0x03000000
	.nb_inplace_divide = (binaryfunc)bw_nil__selfbinary,
#endif /* PY_VERSION_HEX < 0x03000000 */
	.nb_inplace_remainder = (binaryfunc)bw_nil__selfternary,
	.nb_inplace_power = (ternaryfunc)bw_nil__selfternary,
	.nb_inplace_lshift = (binaryfunc)bw_nil__selfbinary,
	.nb_inplace_rshift = (binaryfunc)bw_nil__selfbinary,
	.nb_inplace_and = (binaryfunc)bw_nil__selfbinary,
	.nb_inplace_xor = (binaryfunc)bw_nil__selfbinary,
	.nb_inplace_or = (binaryfunc)bw_nil__selfbinary,

	.nb_floor_divide = (binaryfunc)bw_nil__selfbinary,
	.nb_true_divide = (binaryfunc)bw_nil__selfbinary,
	.nb_inplace_floor_divide = (binaryfunc)bw_nil__selfbinary,
	.nb_inplace_true_divide = (binaryfunc)bw_nil__selfbinary,
};

PyTypeObject
bw_nil_Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"Nil", sizeof(PyObject), 0,
	.tp_dealloc = (destructor)bw_nil__dealloc,
	.tp_repr = (reprfunc)bw_nil__repr,
	.tp_as_number = (PyNumberMethods *)&bw_nil__number,
	.tp_as_sequence = NULL,
	.tp_as_mapping = NULL,
	.tp_hash = NULL,
	.tp_call = NULL,
	.tp_str = (reprfunc)bw_nil__repr,
	.tp_getattro = (getattrofunc)PyObject_GenericGetAttr,
	.tp_setattro = NULL,
	.tp_as_buffer = NULL,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc = "Interoperable Nil object. It is like NULL in SQL statement.",
	.tp_traverse = NULL,
	.tp_clear = NULL,
	.tp_richcompare = (richcmpfunc)bw_nil__richcompare,
};

PyObject*
bw_expectone_at(PyObject* self, PyObject* seq) {
	PyObject* list = NULL;
	PyObject* edge = NULL;
	PyObject* next;
	PyObject* it;

	if ((it = PyObject_GetIter(seq)) == NULL)
		return NULL;
	for (;;) {
		BW_YIELD();
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
				BW_YIELD();
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
				PyErr_SetObject(BwExc_ExpectOneError, list);
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
	Py_DECREF(it);
	return NULL;
}

PyObject*
bw_coalesce_at(PyObject* self, PyObject* seq) {
	PyObject* next;
	PyObject* it;

	if ((it = PyObject_GetIter(seq)) == NULL)
		return NULL;
	for (;;) {
		BW_YIELD();
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

PyObject*
bw_count_at(PyObject* self, PyObject* seq) {
	PyObject* next;
	PyObject* it;
	Py_ssize_t count;

	if ((it = PyObject_GetIter(seq)) == NULL)
		return NULL;
	for (count = 0;;) {
		BW_YIELD();
		if ((next = PyIter_Next(it)) == NULL) {
			if (PyErr_Occurred())
				goto failed;
			break;
		}
		if (next != Py_Nil)
			count++;
		Py_DECREF(next);
	}
	Py_DECREF(it);
	return PyInt_FromSsize_t(count);
failed:
	Py_DECREF(it);
	return NULL;
}


static
PyObject*
bw_diff(PyObject* seq, int op) {
	PyObject* edge = NULL;
	PyObject* next = NULL;
	PyObject* it;
	int cmp;

	if ((it = PyObject_GetIter(seq)) == NULL)
		return NULL;
	for (;;) {
		BW_YIELD();
		if ((edge = PyIter_Next(it)) == NULL) {
			if (PyErr_Occurred())
				goto failed;
			edge = Py_Nil;
			Py_INCREF(edge);
			break;
		}
		if (edge == Py_Nil) {
			Py_DECREF(edge);
			continue;
		}
		for (;;) { // -> edge
			BW_YIELD();
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
	Py_DECREF(it);
	return edge;
failed:
	Py_XDECREF(edge);
	Py_XDECREF(next);
	Py_DECREF(it);
	return NULL;
}

PyObject*
bw_min_at(PyObject* self, PyObject* seq) {
	return bw_diff(seq, Py_LT);
}

PyObject*
bw_min_override_at(PyObject* self, PyObject* seq) {
	return bw_diff(seq, Py_LE);
}

PyObject*
bw_max_at(PyObject* self, PyObject* seq) {
	return bw_diff(seq, Py_GT);
}

PyObject*
bw_max_override_at(PyObject* self, PyObject* seq) {
	return bw_diff(seq, Py_GE);
}

static
PyObject*
bw_mmdiff(PyObject* seq, int lt, int gt) {
	PyObject* min = NULL;
	PyObject* max = NULL;
	PyObject* first = NULL;
	PyObject* second = NULL;
	PyObject* it = NULL;
	int cmp;

	if ((it = PyObject_GetIter(seq)) == NULL)
		return NULL;
need_first1:
	BW_YIELD();
	if ((first = PyIter_Next(it)) == NULL) {
		// 0 elements
		if (PyErr_Occurred())
			goto failed;
		min = Py_Nil;
		Py_INCREF(min);
		max = Py_Nil;
		Py_INCREF(max);
		goto result;
	}
	if (first == Py_Nil) {
		Py_DECREF(first);
		goto need_first1;
	}
need_second1:
	BW_YIELD();
	if ((second = PyIter_Next(it)) == NULL) {
		// 1 elements
		if (PyErr_Occurred())
			goto failed;
		Py_INCREF(first);
		// borrowed refcount one from first
		min = first;
		max = first;
		first = NULL;
		goto result;
	}
	if (second == Py_Nil) {
		Py_DECREF(second);
		goto need_second1;
	}
	if ((cmp = PyObject_RichCompareBool(first, second, lt)) < 0)
		goto failed;
	if (cmp > 0) {
		min = first;
		max = second;
	} else {
		min = second;
		max = first;
	}
	first = second = NULL; // borrowed refcount to min and max
need_first2:
	BW_YIELD();
	if ((first = PyIter_Next(it)) == NULL) {
		// lenght not nils is even
		if (PyErr_Occurred())
			goto failed;
		goto result;
	}
	if (first == Py_Nil) {
		Py_DECREF(first);
		goto need_first2;
	}
need_second2:
	BW_YIELD();
	if ((second = PyIter_Next(it)) == NULL) {
		// lenght not nils is odd, have only first
		if (PyErr_Occurred())
			goto failed;
		if ((cmp = PyObject_RichCompareBool(first, min, lt)) < 0)
			goto failed;
		if (cmp > 0) {
			Py_DECREF(min);
			min = first; // borrowed refcount
		} else {
			if ((cmp = PyObject_RichCompareBool(first, max, gt)) < 0)
				goto failed;
			if (cmp > 0) {
				Py_DECREF(max);
				max = first; // borrowed refcount
			} else
				Py_DECREF(first);
		}
		first = NULL; // borrowed or deleted refcount
		goto result;
	}
	if (second == Py_Nil) {
		Py_DECREF(second);
		goto need_second2;
	}
	if ((cmp = PyObject_RichCompareBool(first, second, lt)) < 0)
		goto failed;
	if (cmp > 0) {
		if ((cmp = PyObject_RichCompareBool(first, min, lt)) < 0)
			goto failed;
		if (cmp > 0) {
			Py_DECREF(min);
			min = first; // borrowed refcount
		} else
			Py_DECREF(first);
		first = NULL; // borrowed or deleted refcount
		if ((cmp = PyObject_RichCompareBool(second, max, gt)) < 0)
			goto failed;
		if (cmp > 0) {
			Py_DECREF(max);
			max = second; // borrowed refcount
		} else
			Py_DECREF(second);
		second = NULL; // borrowed or deleted refcount
	} else {
		if ((cmp = PyObject_RichCompareBool(second, min, lt)) < 0)
			goto failed;
		if (cmp > 0) {
			Py_DECREF(min);
			min = second; // borrowed refcount
		} else
			Py_DECREF(second);
		second = NULL; // borrowed or deleted refcoun
		if ((cmp = PyObject_RichCompareBool(first, max, gt)) < 0)
			goto failed;
		if (cmp > 0) {
			Py_DECREF(max);
			max = first; // borrowed refcount
		} else
			Py_DECREF(first);
		first = NULL; // borrowed or deleted refcount
	}
	goto need_first2;
result:
	Py_DECREF(it);
	return bw_pair_Borrowed(min, max);
failed:
	Py_XDECREF(second);
	Py_XDECREF(first);
	Py_XDECREF(min);
	Py_XDECREF(max);
	Py_DECREF(it);
	return NULL;
}

PyObject*
bw_minmax_at(PyObject* self, PyObject* seq) {
	return bw_mmdiff(seq, Py_LT, Py_GT);
}

PyObject*
bw_minmax_override_at(PyObject* self, PyObject* seq) {
	return bw_mmdiff(seq, Py_LE, Py_GE);
}
