/*
 * (c) Vladimir Yu. Stepanov 2006-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#include "px_types.h"
#include "px_genlink.h"
#include "pxgtc_gentree.h"

#if defined(PxCCTL_IDENT)
static const _u_char rcsid[] = "$Id$";
#endif /* PxCCTL_IDENT */

#if !defined(PxCCTL_NOFUNC)
PxAPI_FUNCDEF(px_genlink_t***)
pxgtc_pathmax(px_genlink_t*** path, px_genlink_t** pp) {
	px_genlink_t* p;
	while ((p = *pp) != NULL) {
		*path++ = pp;
		pp = &p->link[1];
	}
	*path = pp;
	return path;
}
#endif /* !PxCCTL_NOFUNC */
