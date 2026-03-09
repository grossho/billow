/*
 * (c) Vladimir Yu. Stepanov 2010-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(BW_DEBUG_HEADER)
#define BW_DEBUG_HEADER

#if defined(MEMDEBUG)

extern void* PyMem_MallocDebug(Py_ssize_t size, char* file, int lineno);
extern void PyMem_FreeDebug(void* p, char* file, int lineno);

#undef PyMem_MALLOC
#define PyMem_MALLOC(x) PyMem_MallocDebug(x, __FILE__, __LINE__)

#undef PyMem_FREE
#define PyMem_FREE(x) PyMem_FreeDebug(x, __FILE__, __LINE__)

#endif /* MEMDEBUG */

#endif /* BW_DEBUG_HEADER */
