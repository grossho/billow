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
pxgtc_vprev(px_genlink_t*** path, px_genlink_t*** ppp) {
	px_genlink_t** pp;
	px_genlink_t* p;
	if ((p = **ppp) == NULL || *(pp = &p->link[0]) == NULL) {
		px_genlink_t*** ppr = ppp;
		do {
			if (path == ppp) {
				if ((p = **ppr) != NULL)
					*(++ppr) = &p->link[0];
				return ppr;
			}
			pp = *ppp;
			p = **(--ppp);
		} while (&p->link[0] == pp);
	} else {
		*(++ppp) = pp;
		for (p = *pp;;) {
			if ((p = *(pp = &p->link[1])) == NULL)
				break;
			*(++ppp) = pp;
		}
	}
	return ppp;
}
#endif /* !PxCCTL_NOFUNC */
