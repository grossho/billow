/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#include <Python.h>

#if PY_VERSION_HEX >= 0x03000000
int
PyObject_Compare(PyObject* v, PyObject* w) {
	int cmp;
	if ((cmp = PyObject_RichCompareBool(v, w, Py_EQ)) < 0)
		return -2;
	if (cmp)
		return 0;
	if ((cmp = PyObject_RichCompareBool(v, w, Py_GT)) < 0)
		return -2;
	return (cmp > 0) ? 1 : -1;
}
#endif /* PY_VERSION_HEX >= 0x03000000 */
