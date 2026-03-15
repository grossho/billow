/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#include <Python.h>
#include "pycompat.h"
#include "pyexceptionlist.h"
#include "billow.h"
#include "bw_nil.h"
#include "bw_pair.h"
#include "bw_ring.h"
#include "bw_treedict.h"

PyObject* BwExc_ConsistentError;
PyObject* BwExc_ExpectOneError;
PyObject* BwExc_LimitReachedError;
PyObject* BwExc_LockError;
PyObject* BwExc_RDLockError;
PyObject* BwExc_WRLockError;
PyObject* BwExc_FrozenLockError;

struct exceptionlist billow_exceptionlist[] = {
{ "ConsistentError", &BwExc_ConsistentError, &PyExc_Exception, "" },
{ "ExpectOneError", &BwExc_ExpectOneError, &BwExc_ConsistentError, "expect only one value" },
{ "LimitReachedError", &BwExc_LimitReachedError, &BwExc_ConsistentError, "" },
{ "LockError", &BwExc_LockError, &PyExc_Exception, "" },
{ "RDLockError", &BwExc_RDLockError, &BwExc_LockError, "" },
{ "WRLockError", &BwExc_WRLockError, &BwExc_LockError, "" },
{ "FrozenLockError", &BwExc_FrozenLockError, &BwExc_RDLockError, "" },
{ NULL }
};

#define RITM_NIL		0x0001
#define RITM_PAIR		0x0002
#define RITM_RING		0x0004
#define RITM_FLIST		0x0008
#define RITM_TREEDICT		0x0010
#define RITM_ALL		0xffff

#define RITM_FL_ISTYPE		0x01
#define RITM_FL_READY		0x02
#define RITM_MOD_OWN		0x100
#define RITM_MOD_BUILTIN	0x200

struct ritm_struct {
	int ritm_unit;
	int ritm_flags;
	const char* ritm_name;
	PyObject* ritm_object;
};

struct ritm_struct ritm_global[] = {
{ RITM_NIL, 0, "Nil", (PyObject*)&bw_NilStruct },
{ RITM_NIL, RITM_FL_ISTYPE, NULL, (PyObject*)&bw_nil_Type },
{ RITM_RING, RITM_FL_ISTYPE, "ring", (PyObject*)&bw_ring_Type },
{ RITM_PAIR, RITM_FL_ISTYPE, "pair", (PyObject*)&bw_pair_Type },
{ RITM_PAIR, RITM_FL_ISTYPE, NULL, (PyObject*)&bw_pair_iter_Type },
//{ RITM_FLIST, RITM_FL_ISTYPE, "flist", (PyObject*)&bw_flist_Type },
//{ RITM_FLIST, RITM_FL_ISTYPE, NULL, (PyObject*)&bw_flist_iter_Type },
{ RITM_TREEDICT, RITM_FL_ISTYPE, "treedict", (PyObject*)&bw_treedict_Type },
{ RITM_TREEDICT, RITM_FL_ISTYPE, NULL, (PyObject*)&bw_treedict_iter_Type },
{ 0, 0, NULL, NULL }
};

static
int
register_into_module(PyObject* module, int units, int reg) {
	struct ritm_struct* ritm;

	for (ritm = ritm_global; ritm->ritm_unit != 0; ritm++) {
		if (!( ritm->ritm_unit&units ))
			continue;
		// Already done in this module.
		if (ritm->ritm_flags&reg)
			continue;
		ritm->ritm_flags |= reg;
		if (ritm->ritm_flags&RITM_FL_ISTYPE) {
			if (!( ritm->ritm_flags&RITM_FL_READY )) {
				ritm->ritm_flags |= RITM_FL_READY;
				if (PyType_Ready((PyTypeObject*)ritm->ritm_object) < 0)
					return -1;
			}
		}
		if (ritm->ritm_name != NULL) {
			Py_INCREF(ritm->ritm_object);
			if (PyModule_AddObject(module, ritm->ritm_name, ritm->ritm_object) < 0)
				return -1;
		}
	}
	return 0;
}

static
PyObject*
register_into_builtin(PyObject* self, PyObject* arg) {
	PyObject* builtin;
	Py_ssize_t size;
	int status;

	if ((size = PyTuple_Size(arg)) != 0) {
		if (size > 0)
			PyErr_Format(PyExc_TypeError, "takes no arguments (%d given)", size);
		return NULL;
	}
	// COMPAT: __builtin__ vs __builtins__
	if ((builtin = PyImport_ImportModule("__builtin__")) == NULL)
		return NULL;
	status = register_into_module(builtin, RITM_ALL, RITM_MOD_BUILTIN);
	Py_DECREF(builtin);

	if (status < 0)
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef billow_methods[] = {
// from: bw_nil.c
{ "min_at", (PyCFunction)bw_min_at, METH_VARARGS, NULL },
{ "min_it", (PyCFunction)bw_min_at, METH_O, NULL },
{ "min", (PyCFunction)bw_min_at, METH_O, NULL },

{ "min_override_at", (PyCFunction)bw_min_override_at, METH_VARARGS, NULL },
{ "min_override_it", (PyCFunction)bw_min_override_at, METH_O, NULL },
{ "min_override", (PyCFunction)bw_min_override_at, METH_O, NULL },

{ "max_at", (PyCFunction)bw_max_at, METH_VARARGS, NULL },
{ "max_it", (PyCFunction)bw_max_at, METH_O, NULL },
{ "max", (PyCFunction)bw_max_at, METH_O, NULL },

{ "max_override_at", (PyCFunction)bw_max_at, METH_VARARGS, NULL },
{ "max_override_it", (PyCFunction)bw_max_override_at, METH_O, NULL },
{ "max_override", (PyCFunction)bw_max_override_at, METH_O, NULL },

{ "minmax_at", (PyCFunction)bw_minmax_at, METH_VARARGS, NULL },
{ "minmax_it", (PyCFunction)bw_minmax_at, METH_O, NULL },
{ "minmax", (PyCFunction)bw_minmax_at, METH_O, NULL },

{ "minmax_override_at", (PyCFunction)bw_minmax_override_at, METH_VARARGS, NULL },
{ "minmax_override_it", (PyCFunction)bw_minmax_override_at, METH_O, NULL },
{ "minmax_override", (PyCFunction)bw_minmax_override_at, METH_O, NULL },

{ "expectone", (PyCFunction)bw_expectone_at, METH_VARARGS, NULL },
{ "coalesce", (PyCFunction)bw_coalesce_at, METH_VARARGS, NULL },
{ "count", (PyCFunction)bw_count_at, METH_VARARGS, NULL },

{ "expectone_at", (PyCFunction)bw_expectone_at, METH_VARARGS, NULL },
{ "coalesce_at", (PyCFunction)bw_coalesce_at, METH_VARARGS, NULL },
{ "count_at", (PyCFunction)bw_count_at, METH_VARARGS, NULL },

{ "expectone_it", (PyCFunction)bw_expectone_at, METH_O, NULL },
{ "coalesce_it", (PyCFunction)bw_coalesce_at, METH_O, NULL },
{ "count_it", (PyCFunction)bw_count_at, METH_O, NULL },

// from: bw_pair.c
{ "zippair", (PyCFunction)bw_zippair, METH_VARARGS, NULL },

// from: bw_treedict.c
{ "cmp_buffer", (PyCFunction)bw_cmp_buffer, METH_VARARGS, NULL },
#if PY_VERSION_HEX >= 0x03000000
{ "cmp_unicode", (PyCFunction)bw_cmp_unicode, METH_VARARGS, NULL },
{ "cmp_long", (PyCFunction)bw_cmp_long, METH_VARARGS, NULL },
#endif /* PY_VERSION_HEX >= 0x03000000 */

// from: bw_billow.c
{ "register_into_builtin", (PyCFunction)register_into_builtin, METH_VARARGS, NULL },

{ NULL, NULL, 0, NULL }
};

PyDoc_STRVAR(billow_module_documentation,
"");

#if PY_VERSION_HEX < 0x03000000
PyMODINIT_FUNC
initbillow(void) {
	PyObject* module;
	PyObject* builtin;

	if ((module = Py_InitModule4("billow", billow_methods, billow_module_documentation, NULL, PYTHON_API_VERSION)) == NULL
	 || exceptions_into_module(module, billow_exceptionlist, "billow") < 0
	 || register_into_module(module, RITM_ALL, RITM_MOD_OWN) < 0)
		return;

	if ((builtin = PyImport_ImportModule("__builtin__")) != NULL) {
		register_into_module(builtin, RITM_NIL, RITM_MOD_BUILTIN);
		Py_DECREF(builtin);
	}
}
#else /* 0x03000000 */
static struct PyModuleDef billow_module = {
	PyModuleDef_HEAD_INIT,
	"billow",
	billow_module_documentation,
	-1,
	billow_methods,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC
PyInit_billow(void) {
	PyObject* module;
	PyObject* builtin = NULL;

	if ((module = PyModule_Create(&billow_module)) == NULL
	 || exceptions_into_module(module, billow_exceptionlist, "billow") < 0
	 || register_into_module(module, RITM_ALL, RITM_MOD_OWN) < 0
	 || (builtin = PyImport_ImportModule("builtins")) == NULL
	 || register_into_module(builtin, RITM_ALL, RITM_MOD_BUILTIN) < 0) {
		Py_XDECREF(builtin);
		Py_CLEAR(module);
	}
	return module;
}
#endif /* 0x03000000 */
