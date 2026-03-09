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

#if !defined(PxCCTL_NOGLOB)
#endif /* !PxCCTL_NOGLOB */

#if !defined(PxCCTL_NOFUNC)
PxAPI_FUNCDEF(px_genlink_t*)
pxgtc_selectmin(px_genlink_t* root) {
	if (root == NULL)
		return NULL;
	for (;;) {
		px_genlink_t* t = root->link[0];
		if (t == NULL)
			break;
		root = t;
	}
	return root;
}
#endif /* !PxCCTL_NOFUNC */
