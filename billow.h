/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(HEADER_BILLOW)
#define HEADER_BILLOW

#include <Python.h>

#include "pycompat.h"
#include "px_types.h"
#include "bw_pair.h"
#include "bw_ring.h"
//#include "bw_flist.h"
#include "bw_treedict.h"

#if !defined(c_cast)
#define c_cast(__struct,__field,__ptr)					\
	((__struct*)&(((char*)__ptr)[(char*)0 - (char*)&((__struct*)0)->__field]))
#endif /* c_cast */

extern PyObject* BwExc_LockError;
extern PyObject* BwExc_RDLockError;
extern PyObject* BwExc_WRLockError;
extern PyObject* BwExc_FrozenLockError;
extern PyObject* BwExc_LimitReachedError;
extern PyObject* BwExc_ExpectOneError;
extern PyObject* BwExc_ConsistentError;

#endif /* HEADER_BILLOW */
