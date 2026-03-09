/*
 * (c) Vladimir Yu. Stepanov 2006-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(HEADER_PXGTC_BALANCE)
#define HEADER_PXGTC_BALANCE

#include "px_types.h"
#include "px_genlink.h"

struct __pxgtc_algo_t;

typedef _u3 pxgtc_typeof_getstate(const struct __pxgtc_algo_t* algo, px_genlink_t* p);
typedef void pxgtc_typeof_setstate(const struct __pxgtc_algo_t* algo, px_genlink_t* p, _u3 state);
typedef _u5 pxgtc_typeof_getw5(const struct __pxgtc_algo_t* algo, px_genlink_t* p);
typedef void pxgtc_typeof_setw5(const struct __pxgtc_algo_t* algo, px_genlink_t* p, _u5 set);
typedef _u6 pxgtc_typeof_getw6(const struct __pxgtc_algo_t* algo, px_genlink_t* p);
typedef void pxgtc_typeof_setw6(const struct __pxgtc_algo_t* algo, px_genlink_t* p, _u6 set);
typedef void pxgtc_typeof_balance(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);

typedef struct __pxgtc_algo_t {
	pxgtc_typeof_getstate* algo_getstate;
	pxgtc_typeof_setstate* algo_setstate;
	union {
		pxgtc_typeof_getw5* algo_getw5;
		pxgtc_typeof_getw6* algo_getw6;
	};
	union {
		pxgtc_typeof_setw5* algo_setw5;
		pxgtc_typeof_setw6* algo_setw6;
	};
	pxgtc_typeof_balance* algo_balance_insert;
	pxgtc_typeof_balance* algo_balance_remove;
} pxgtc_algo_t;

PxAPI_FUNCEXT(void) pxgtc_balance_avl_insert(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);
PxAPI_FUNCEXT(void) pxgtc_balance_avl_remove(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);
PxAPI_FUNCEXT(void) pxgtc_balance_rb_insert(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);
PxAPI_FUNCEXT(void) pxgtc_balance_rb_remove(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);

// Traversal in-order )tio)
PxAPI_FUNCEXT(px_genlink_t*)pxgtc_tio5_selectinc(const struct __pxgtc_algo_t* algo, px_genlink_t* p, _u5 i);
PxAPI_FUNCEXT(px_genlink_t*)pxgtc_tio5_selectdec(const struct __pxgtc_algo_t* algo, px_genlink_t* p, _u5 i);
PxAPI_FUNCEXT(px_genlink_t***)pxgtc_tio5_pathinc(const struct __pxgtc_algo_t* algo, px_genlink_t*** ppp, px_genlink_t** pp, _u5 i);
PxAPI_FUNCEXT(px_genlink_t***)pxgtc_tio5_pathdec(const struct __pxgtc_algo_t* algo, px_genlink_t*** ppp, px_genlink_t** pp, _u5 i);
PxAPI_FUNCEXT(_u5)pxgtc_tio5_rankinc(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);
PxAPI_FUNCEXT(_u5)pxgtc_tio5_rankdec(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);

PxAPI_FUNCEXT(px_genlink_t*)pxgtc_tio6_selectinc(const struct __pxgtc_algo_t* algo, px_genlink_t* p, _u6 i);
PxAPI_FUNCEXT(px_genlink_t*)pxgtc_tio6_selectdec(const struct __pxgtc_algo_t* algo, px_genlink_t* p, _u6 i);
PxAPI_FUNCEXT(px_genlink_t***)pxgtc_tio6_pathinc(const struct __pxgtc_algo_t* algo, px_genlink_t*** ppp, px_genlink_t** pp, _u6 i);
PxAPI_FUNCEXT(px_genlink_t***)pxgtc_tio6_pathdec(const struct __pxgtc_algo_t* algo, px_genlink_t*** ppp, px_genlink_t** pp, _u6 i);
PxAPI_FUNCEXT(_u6)pxgtc_tio6_rankinc(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);
PxAPI_FUNCEXT(_u6)pxgtc_tio6_rankdec(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);


PxAPI_FUNCEXT(void) pxgtc_balance_tio5avl_insert(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);
PxAPI_FUNCEXT(void) pxgtc_balance_tio5avl_remove(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);
PxAPI_FUNCEXT(void) pxgtc_balance_tio5rb_insert(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);
PxAPI_FUNCEXT(void) pxgtc_balance_tio5rb_remove(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);

PxAPI_FUNCEXT(void) pxgtc_balance_tio6avl_insert(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);
PxAPI_FUNCEXT(void) pxgtc_balance_tio6avl_remove(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);
PxAPI_FUNCEXT(void) pxgtc_balance_tio6rb_insert(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);
PxAPI_FUNCEXT(void) pxgtc_balance_tio6rb_remove(const struct __pxgtc_algo_t* algo, px_genlink_t*** path, px_genlink_t*** ppp);

#define PxGTC_MAXPATH			48


#define PxGTC_FIND(__cmptype, __cmpfunc, __root, __entry)		\
	while ((__root) != NULL) {					\
		__cmptype __cmp = __cmpfunc((__root), (__entry));	\
		if (__cmp == 0)						\
			break;						\
		__root = (__root)->link[__cmp > 0];			\
	}

#define PxGTC_OO_FIND(__cmptype, __cmpfunc, __ob, __root, __entry)	\
	while ((__root) != NULL) {					\
		__cmptype __cmp = __cmpfunc((__ob), (__root), (__entry)); \
		if (__cmp == 0)						\
			break;						\
		__root = (__root)->link[__cmp > 0];			\
	}

#define PxGTC_SEARCH(__base, __cursor, __cmptype, __cmpfunc, __proot, __entry) \
	do {								\
		px_genlink_t** __pp;					\
		__cursor = __base;					\
		__pp = __proot;						\
		while (*(__pp) != NULL) {				\
			__cmptype __cmp = __cmpfunc(*(__pp), (__entry)); \
			if (__cmp == 0)					\
				break;					\
			*(__cursor)++ = __pp;				\
			__pp = &(*(__pp))->link[__cmp > 0];		\
		}							\
		*(__cursor) = (__pp);					\
	} while (0)

#define PxGTC_OO_SEARCH(__base, __cursor, __cmptype, __cmpfunc, __ob, __proot, __entry) \
	do {								\
		px_genlink_t** __pp;					\
		__cursor = __base;					\
		__pp = __proot;						\
		while (*(__pp) != NULL) {				\
			__cmptype __cmp = __cmpfunc((__ob), *(__pp), (__entry)); \
			if (__cmp == 0)					\
				break;					\
			*(__cursor)++ = __pp;				\
			__pp = &(*(__pp))->link[__cmp > 0];		\
		}							\
		*(__cursor) = (__pp);					\
	} while (0)

#endif /* HEADER_PXGTC_BALANCE */
