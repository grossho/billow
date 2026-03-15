/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(HEADER_PYCOMPAT)
#define HEADER_PYCOMPAT

#include <Python.h>

#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)

typedef int Py_ssize_t;

#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN

#define PyInt_FromSsize_t(x) PyInt_FromLong(x)
#define PyInt_AsSsize_t(x) PyInt_AsLong(x)
#define PyIndex_Check(x) PyInt_Check(x)
#define PyNumber_AsSsize_t(x,e) (PyInt_Check(x) ? PyInt_AS_LONG(x) : (PyErr_SetNone(e), -1))

typedef Py_ssize_t (*lenfunc)(PyObject *);
typedef PyObject *(*ssizeargfunc)(PyObject *, Py_ssize_t);
typedef PyObject *(*ssizessizeargfunc)(PyObject *, Py_ssize_t, Py_ssize_t);
typedef int(*ssizeobjargproc)(PyObject *, Py_ssize_t, PyObject *);
typedef int(*ssizessizeobjargproc)(PyObject *, Py_ssize_t, Py_ssize_t, PyObject *);

#endif /* PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN) */

#if PY_VERSION_HEX < 0x02060100
#define Py_TYPE(ob) (((PyObject*)(ob))->ob_type)
#define Py_SIZE(ob) (((PyVarObject*)(ob))->ob_size)
#endif /* PY_VERSION_HEX < 0x02060100 */

#if PY_VERSION_HEX < 0x02050000
#define Py_ssize_t_FROM_FMT	"%d"
#define Py_ssize_t_PARSE_FMT	"i"
#else
#define Py_ssize_t_FROM_FMT	"%zd"
#define Py_ssize_t_PARSE_FMT	"n"
#endif /* PY_VERSION_HEX < 0x02060100 */

#if PY_VERSION_HEX < 0x02060000

#if !defined(PyVarObject_HEAD_INIT)
#define PyVarObject_HEAD_INIT(type, size)   PyObject_HEAD_INIT(type) size,
#endif /* !defined(PyVarObject_HEAD_INIT) */

#define PyObject_Bytes PyObject_Str

#define PyBytes_Type PyString_Type
#define PyBytes_Check PyString_Check
#define PyBytes_CheckExact PyString_CheckExact
#define PyBytes_FromString PyString_FromString
#define PyBytes_FromStringAndSize PyString_FromStringAndSize
#define PyBytes_FromFormat PyString_FromFormat
#define PyBytes_FromFormatV PyString_FromFormatV
#define PyBytes_Size PyString_Size
#define PyBytes_GET_SIZE PyString_GET_SIZE
#define PyBytes_AsString PyString_AsString
#define PyBytes_AS_STRING PyString_AS_STRING
#define PyBytes_Concat PyString_Concat
#define PyBytes_ConcatAndDel PyString_ConcatAndDel
#define _PyBytes_Resize _PyString_Resize
#define PyBytes_Format PyString_Format
#define PyBytes_InternInPlace PyString_InternInPlace
#define PyBytes_Decode PyString_Decode
#define PyBytes_AsDecodedObject PyString_AsDecodedObject
#define PyBytes_Encode PyString_Encode
#define PyBytes_AsEncodedObject PyString_AsEncodeddObject


#define PyMessage_RequiredAnBytes(x) x " required an str()"


#else /* PY_VERSION_HEX < 0x02060000 */


#define PyMessage_RequiredAnBytes(x) x " required an bytes()"


#endif /* PY_VERSION_HEX < 0x02060000 */


#if PY_VERSION_HEX >= 0x03000000

#define PyString_AS_STRING PyUnicode_AS_UNICODE
#define PyString_Check PyUnicode_Check
#define PyString_CheckExact PyUnicode_CheckExact
#define PyString_FromFormat PyUnicode_FromFormat
#define PyString_FromString PyUnicode_FromString
#define PyString_FromStringAndSize PyUnicode_FromStringAndSize
#define PyString_GET_SIZE PyUnicode_GET_SIZE
#define PyString_InternFromString PyUnicode_InternFromString
#define PyString_InternInPlace PyUnicode_InternInPlace
#define _PyString_Join PyUnicode_Join
#define PyString_AsString PyUnicode_AsLatin1String

#define PyInt_AsLong PyLong_AsLong
#define PyInt_AsSsize_t PyLong_AsSsize_t
#define PyInt_FromLong PyLong_FromLong
#define PyInt_FromSsize_t PyLong_FromSsize_t
#define PyInt_Check PyLong_Check
extern int PyObject_Compare(PyObject* v, PyObject* w);

#endif /* PY_VERSION_HEX >= 0x03000000 */

#if PY_VERSION_HEX >= 0x03030000 && PY_VERSION_HEX < 0x030e0000
#define	PyUnicodeWriter _PyUnicodeWriter
#define PyUnicodeWriter_WriteChar _PyUnicodeWriter_WriteChar
#define PyUnicodeWriter_WriteASCII _PyUnicodeWriter_WriteASCIIString
#define PyUnicodeWriter_WriteChar _PyUnicodeWriter_WriteChar
#define PyUnicodeWriter_WriteStr _PyUnicodeWriter_WriteStr
#define PyUnicodeWriter_Finish _PyUnicodeWriter_Finish
#define PyUnicodeWriter_Discard _PyUnicodeWriter_Dealloc
#endif /*  PY_VERSION_HEX >= 0x03030000 && PY_VERSION_HEX < 0x030e0000 */

#if !defined(Py_TYPE)
#define Py_TYPE(ob)             (((PyObject*)(ob))->ob_type)
#endif /* !defined(Py_TYPE) */

#if PY_VERSION_HEX < 0x03000000

extern void bw_tick_yield(void);
#define BW_YIELD() if (--_Py_Ticker < 0) bw_tick_yield();

#else /* PY_VERSION_HEX < 0x03000000 */

#define BW_YIELD()

#endif /* PY_VERSION_HEX < 0x03000000 */

#endif /* HEADER_PYCOMPAT */
