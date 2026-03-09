/*
 * (c) Vladimir Yu. Stepanov 2006-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#include "px_types.h"
#include "px_genlink.h"
#include "pxgtc_gentree.h"

#if defined(COMPILE_IDENT)
static const _u_char rcsid[] = "$Id$";
#endif /* COMPILE_IDENT */

#if defined(PxCCTL_IDENT)
static const _u_char rcsid[] = "$Id$";
#endif /* PxCCTL_IDENT */

#if !defined(PxCCTL_NOGLOB)
#endif /* !PxCCTL_NOGLOB */

#if !defined(PxCCTL_NOFUNC)
PxAPI_FUNCDEF(void)
pxgtc2qcl_next(px_genlink_t*** path, px_genlink_t** pp, px_genlink_t* qcl) {
	px_genlink_t*** ppp = path;
	px_genlink_t* s = qcl;
	px_genlink_t* p;
	for (;;) {
		for (; (p = *(*ppp = pp)) != NULL; ppp++)
			pp = &p->link[0];
		if (ppp == path)
			break;
		p = *(pp = *(--ppp));
		*pp = p->link[1];
		p->link[0] = s;
		s->link[1] = p;
		s = p;
	}
	qcl->link[0] = s;
	s->link[1] = qcl;
}
#endif /* !PxCCTL_NOFUNC */
