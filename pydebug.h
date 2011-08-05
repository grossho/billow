/*
 * Copyright 2011 Vladimir Yu. Stepanov
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 * 
 *    1. Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright notice, this list
 *       of conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY AUTHOR ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL AUTHOR OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if !defined(PYDEBUG_HEADER)
#define PYDEBUG_HEADER


#if MEMDEBUG

void* PyMem_MallocDebug(Py_ssize_t size, int lineno) {
	void* p = PyMem_Malloc(size);
	fprintf(stderr, "MALLOC: %p: size=%lld, line %d\n", p, (long long)size, lineno);
	return p;
}

void PyMem_FreeDebug(void* p, int lineno) {
	fprintf(stderr, "FREE:   %p: line %d\n", p, lineno);
	PyMem_Free(p);
}

#undef PyMem_MALLOC
#define PyMem_MALLOC(x) PyMem_MallocDebug(x, __LINE__)

#undef PyMem_FREE
#define PyMem_FREE(x) PyMem_FreeDebug(x, __LINE__)

#endif /* MEMDEBUG */


#endif /* PYDEBUG_HEADER */
