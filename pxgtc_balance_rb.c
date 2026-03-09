/*
 * (c) Vladimir Yu. Stepanov 2006-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#include "px_types.h"
#include "px_genlink.h"
#include "pxgtc_gentree.h"
#include "pxgtc_balance.h"

#if defined(PxCCTL_IDENT)
static const _u_char rcsid[] = "$Id$";
#endif /* PxCCTL_IDENT */

#if !defined(PxCCTL_NOFUNC)
PxAPI_FUNCDEF(void)
pxgtc_balance_rb_insert(const pxgtc_algo_t* algo,
		px_genlink_t*** path, px_genlink_t*** ppp) {
	px_genlink_t** pp;
	px_genlink_t* p;
	px_genlink_t** ps;
	px_genlink_t* s;
	px_genlink_t** pn;
	px_genlink_t* n;
	bool i;
	bool r;
	p = **ppp;
	p->link[1] = p->link[0] = NULL;
	algo->algo_setstate(algo, p, 0);
	for (path += 2; ppp >= path;) {
		pn = *ppp;
		p = *(pp = *(--ppp));
		if ((algo->algo_getstate(algo, p) & 1) != 0)
			break;
		s = *(ps = *(--ppp));
		algo->algo_setstate(algo, s, 0);
		r = (&s->link[0] == pp);
		if ((n = s->link[r]) != NULL && (algo->algo_getstate(algo, n) & 1) == 0) {
			algo->algo_setstate(algo, n, 1);
			algo->algo_setstate(algo, p, 1);
		} else {
			if (&p->link[r] == pn) {
				i = !r;
				*pp = n = *pn;
				*pn = n->link[i];
				n->link[i] = p;
				p = n;
			}
			algo->algo_setstate(algo, p, 1);
			*ps = p;
			*pp = *(pn = &p->link[r]);
			*pn = s;
			break;
		}
	}
	p = **(path - 2);
	algo->algo_setstate(algo, p, 1);
}

PxAPI_FUNCDEF(void)
pxgtc_balance_rb_remove(const pxgtc_algo_t* algo,
		px_genlink_t*** path, px_genlink_t*** ppp) {
	px_genlink_t** pp;
	px_genlink_t* p;
	px_genlink_t** ps;
	px_genlink_t* s;
	px_genlink_t** pn;
	px_genlink_t* n;
	_u3 i;
	_u3 r;
	_u3 c;

	p = *(pp = *ppp);
	if ((s = *(ps = &p->link[1])) == NULL)
		*pp = p->link[0];
	else {
		if (s->link[0] == NULL) {
			*pp = s;
			s->link[0] = p->link[0];
			*(++ppp) = &s->link[1];
		} else {
			px_genlink_t*** ppt = ++ppp;
			for (;;) {
				px_genlink_t** pt = &(*ps)->link[0];
				if (*pt == NULL)
					break;
				*(++ppp) = ps = pt;
			}
			*pp = s = *ps;
			*ps = *(*ppt = &s->link[1]);
			s->link[0] = p->link[0];
			s->link[1] = p->link[1];
		}
		c = algo->algo_getstate(algo, s) & 1;
		algo->algo_setstate(algo, s, algo->algo_getstate(algo, p) & 1);
		algo->algo_setstate(algo, p, c);
	}
	if (algo->algo_getstate(algo, p) & 1)
	for (;;) {
		if ((p = *(pp = *ppp)) != NULL && (algo->algo_getstate(algo, p) & 1) == 0) {
			algo->algo_setstate(algo, p, 1);
			break;
		}
		if (--ppp < path)
			break;
		s = *(ps = *ppp);
		r = !(i = (&s->link[0] != pp));
		n = *(pn = &s->link[r]);
		if (!(algo->algo_getstate(algo, n) & 1)) {
			algo->algo_setstate(algo, n, 1);
			algo->algo_setstate(algo, s, 0);
			*ps = n;
			*pn = n = *(ps = &n->link[i]);
			*ps = s;
			*(++ppp) = ps;
		}
		if ((p = n->link[r]) == NULL || (algo->algo_getstate(algo, p) & 1) != 0) {
			if ((p = *(pp = &n->link[i])) == NULL || (algo->algo_getstate(algo, p) & 1) != 0) {
				algo->algo_setstate(algo, n, 0);
				continue;
			}
			*pp = p->link[r];
			p->link[r] = n;
			*pn = n = p;
		}
		algo->algo_setstate(algo, n, (algo->algo_getstate(algo, s) & 1));
		algo->algo_setstate(algo, s, 1);
		algo->algo_setstate(algo, n->link[r], 1);
		*pn = *(pp = &n->link[i]);
		*pp = s;
		*ps = n;
		break;
	}
	if ((p = **path) != NULL)
		algo->algo_setstate(algo, p, 1);
}
#endif /* PxCCTL_NOFUNC */
