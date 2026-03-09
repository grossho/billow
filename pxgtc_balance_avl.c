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
pxgtc_balance_avl_insert(const pxgtc_algo_t* algo,
		px_genlink_t*** path, px_genlink_t*** ppp) {
	px_genlink_t** pp;
	px_genlink_t* p;
	px_genlink_t** ps;
	px_genlink_t* s;
	_u3 c;
	_u3 i;
	_u3 r;

	p = *(pp = *ppp);
	p->link[1] = p->link[0] = NULL;
	algo->algo_setstate(algo, p, 0);
	while (ppp != path) {
		ps = pp;
		p = *(pp = *(--ppp));
		r = (&p->link[0] == ps);
		c = algo->algo_getstate(algo, p);
		algo->algo_setstate(algo, p, (((c + 1)<<r)&3)>>c);
		if (!(c &= 3))
			continue;
		i = !r;
		if (c & (1<<i))
			break;
		s = *ps;
		if (algo->algo_getstate(algo, s)&(1<<r)) {
			*pp = s;
			*ps = s->link[r];
			s->link[r] = p;
			algo->algo_setstate(algo, p, 0);
			algo->algo_setstate(algo, s, 0);
		} else {
			p = s->link[r];
			*ps = p->link[r];
			p->link[r] = *pp;
			*pp = p;
			s->link[r] = p->link[i];
			p->link[i] = s;
			c = algo->algo_getstate(algo, p);
			algo->algo_setstate(algo, p, 0);
			algo->algo_setstate(algo, p->link[0], (c&1)<<1);
			algo->algo_setstate(algo, p->link[1], (c>>1)&1);
		}
		break;
	}
}

PxAPI_FUNCDEF(void)
pxgtc_balance_avl_remove(const pxgtc_algo_t* algo,
		px_genlink_t*** path, px_genlink_t*** ppp) {
	px_genlink_t** pp;
	px_genlink_t* p;
	px_genlink_t** ps;
	px_genlink_t* s;
	_u3 c;
	_u3 i;
	_u3 r;

	p = *(pp = *ppp);
	if (p->link[0] == NULL)
		*pp = p->link[1];
	else
	if (p->link[1] == NULL)
		*pp = p->link[0];
	else {
		px_genlink_t*** ppt = ++ppp;
		r = !(i = (algo->algo_getstate(algo, p)>>1)&1);
		s = *(ps = &p->link[r]);
		if (s->link[i] == NULL) {
			*pp = s;
			s->link[i] = p->link[i];
			pp = &s->link[r];
		} else {
			for (;;) {
				px_genlink_t** pt = &(*ps)->link[i];
				if (*pt == NULL)
					break;
				*(++ppp) = ps = pt;
			}
			*pp = s = *ps;
			*ps = *(*ppt = &s->link[r]);
			s->link[0] = p->link[0];
			s->link[1] = p->link[1];
			pp = ps;
		}
		algo->algo_setstate(algo, s, algo->algo_getstate(algo, p)&3);
	}
	while (ppp != path) {
		ps = pp;
		p = *(pp = *(--ppp));
		i = (&p->link[0] != ps);
		c = algo->algo_getstate(algo, p)&3;
		algo->algo_setstate(algo, p, (((c + 1)<<i)&3)>>c);
		if (c == 0)
			break;
		r = !i;
		if (c&(1<<r))
			continue;
		s = *(ps = &p->link[r]);
		c = algo->algo_getstate(algo, s)&3;
		algo->algo_setstate(algo, s, (((c + 1)<<r)&3)>>c);
		if (c&(1<<r)) {
			p = s->link[i];
			*ps = p->link[i];
			p->link[i] = *pp;
			*pp = p;
			s->link[i] = p->link[r];
			p->link[r] = s;
			c = algo->algo_getstate(algo, p);
			algo->algo_setstate(algo, p, 0);
			algo->algo_setstate(algo, p->link[0], (c&1)<<1);
			algo->algo_setstate(algo, p->link[1], (c>>1)&1);
		} else {
			*pp = s;
			*ps = s->link[i];
			s->link[i] = p;
			c ^= 1<<i;
			algo->algo_setstate(algo, p, c);
			if (c != 0)
				break;
		}
	}
}
#endif /* !PxCCTL_NOFUNC */
