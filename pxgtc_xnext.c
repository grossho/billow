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
pxgtc_xnext(px_genlink_t*** path, px_genlink_t*** ppp) {
	px_genlink_t** pp;
	px_genlink_t* p;
	if (ppp == NULL)
		return path;
	if ((p = **ppp) == NULL) {
		for (;;) {
			pp = *ppp;
			if (--ppp < path)
				return NULL;
			p = **ppp;
			if (&p->link[0] == pp) {
				*(++ppp) = &p->link[1];
				return ppp;
			}
		}
	} else {
		*(++ppp) = &p->link[0];
		return ppp;
	}
}
#endif /* !PxCCTL_NOFUNC */
