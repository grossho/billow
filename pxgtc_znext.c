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
pxgtc_znext(px_genlink_t*** path, px_genlink_t*** ppp) {
	px_genlink_t** pp;
	px_genlink_t* p;
	if (path == ppp)
		return NULL;
	path = ppp;
	pp = &(**(--ppp))->link[1];
	if (pp != *path && *pp != NULL)
	for (;;) {
		*(++ppp) = pp;
		p = *pp;
		pp = &p->link[0];
		if (*pp == NULL) {
			pp = &p->link[1];
			if (*pp == NULL)
				break;
		}
	}
	return ppp;
}
#endif /* !PxCCTL_NOFUNC */
