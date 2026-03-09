/*
 * (c) Vladimir Yu. Stepanov 2010-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#include <Python.h>
#include "bw_debug.h"

#if defined(MEMDEBUG)
int current_mallocs = 0;
void* PyMem_MallocDebug(Py_ssize_t size, char* file, int lineno) {
	void* p;
	current_mallocs += 1;
	switch (current_mallocs) {
	case 25:
		p = NULL;
		break;
	default:
		p = PyMem_Malloc(size);
		break;
	}
	fprintf(stderr, "%s: MALLOC: %p: size=%lld, line %d, mallocs=%d\n", file, p, (long long)size, lineno, current_mallocs);
	fflush(stderr);
	return p;
}

void PyMem_FreeDebug(void* p, char* file, int lineno) {
	PyMem_Free(p);
	fprintf(stderr, "%s: FREE:   %p: line %d\n", file, p, lineno);
	fflush(stderr);
}
#endif /* defined(MEMDEBUG) */
