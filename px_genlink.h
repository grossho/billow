/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(HEADER_PX_GENLINK)
#define HEADER_PX_GENLINK

#include "px_types.h"

#if 0
typedef struct __px_genlink_t {
	struct __px_genlink_t* link[2];
} px_genlink_t;
#else
typedef union __px_genlink_t {
	union __px_genlink_t* link[2];
	void* voidptr[2];
	_u_char* u_char[2];
	_s_char* s_char[2];
	_u_ptr u_ptr[2];
	_s_ptr s_ptr[2];
} px_genlink_t;
#endif

#define PxQCL_nullable(__x0)						\
	do {								\
		register px_genlink_t* __x = (__x0);			\
		__x->link[0] = NULL;					\
		__x->link[1] = NULL;					\
	} while (0)

#define PxQCL_selflinked(__x0)						\
	do {								\
		register px_genlink_t* __x = (__x0);			\
		__x->link[0] = __x;					\
		__x->link[1] = __x;					\
	} while (0)

#define PxQCL_selfremove(__x0, __o0)					\
	do {								\
		register bool __o = !!(__o0);				\
		register px_genlink_t* __x = (__x0);			\
		__x->link[!__o]->link[+__o] = __x->link[+__o];		\
		__x->link[+__o]->link[!__o] = __x->link[!__o];		\
	} while (0)

#define PxQCL_selfinsert_after(__x0, __y0, __o0)			\
	do {								\
		register bool __o = !!(__o0);				\
		register px_genlink_t* __x = (__x0);			\
		register px_genlink_t* __y = (__y0);			\
		register px_genlink_t* __z = __y->link[+__o];		\
		__x->link[+__o] = __z;					\
		__x->link[!__o] = __y;					\
		__z->link[!__o] = __x;					\
		__y->link[+__o] = __x;					\
	} while (0)

#define PxQCL_selfinsert_before(__x0, __y0, __o0)			\
	do {								\
		register bool __o = !!(__o0);				\
		register px_genlink_t* __x = (__x0);			\
		register px_genlink_t* __y = (__y0);			\
		register px_genlink_t* __z = __y->link[!__o];		\
		__x->link[!__o] = __z;					\
		__x->link[+__o] = __y;					\
		__z->link[+__o] = __x;					\
		__y->link[!__o] = __x;					\
	} while (0)

// merge: append any double linked list.
#define PxQCL_merge(__x0, __z0, __o0)					\
	do {								\
		if (!!(__o0))						\
			PxQCL_merge_lo(__x0, __z0);			\
		else							\
			PxQCL_merge_hi(__x0, __z0);			\
	} while (0)

#define PxQCL_merge_lo(__x0, __z0) 					\
	do {								\
		register px_genlink_t* __z = (__z0);			\
		if (__z != __z->link[0]) {				\
			register px_genlink_t* __x = (__x0);		\
			register px_genlink_t* __y = __x->link[0];	\
			(__x->link[0] = __z->link[0])->link[1] = __x;	\
			(__y->link[1] = __z->link[1])->link[0] = __y;	\
		}							\
	} while (0)

#define PxQCL_merge_hi(__x0, __z0) 					\
	do {								\
		register px_genlink_t* __z = (__z0);			\
		if (__z != __z->link[0]) {				\
			register px_genlink_t* __x = (__x0);		\
			register px_genlink_t* __y = __x->link[1];	\
			(__x->link[1] = __z->link[1])->link[0] = __x;	\
			(__y->link[0] = __z->link[0])->link[1] = __y;	\
		}							\
	} while (0)

// extend: append non-empty double linked list.
#define PxQCL_extend(__x0, __z0, __o0)					\
	do {								\
		if (!!(__o0))						\
			PxQCL_extend_lo(__x0, __z0);			\
		else							\
			PxQCL_extend_hi(__x0, __z0);			\
	} while (0)

#define PxQCL_extend_lo(__x0, __z0) 					\
	do {								\
		register px_genlink_t* __x = (__x0);			\
		register px_genlink_t* __z = (__z0);			\
		register px_genlink_t* __y = __x->link[0];		\
		(__x->link[0] = __z->link[0])->link[1] = __x;		\
		(__y->link[1] = __z->link[1])->link[0] = __y;		\
	} while (0)

#define PxQCL_extend_hi(__x0, __z0) 					\
	do {								\
		register px_genlink_t* __x = (__x0);			\
		register px_genlink_t* __z = (__z0);			\
		register px_genlink_t* __y = __x->link[1];		\
		(__x->link[1] = __z->link[1])->link[0] = __x;		\
		(__y->link[0] = __z->link[0])->link[1] = __y;		\
	} while (0)

#define PxQCL_in(__x0, __z0, __o0)					\
	do {								\
		if (!!(__o0))						\
			PxQCL_in_lo(__x0, __z0);			\
		else							\
			PxQCL_in_hi(__x0, __z0);			\
	} while (0)

#define PxQCL_in_lo(__x0, __z0)						\
	do {								\
		register px_genlink_t* __x = (__x0);			\
		register px_genlink_t* __y = __x->link[0];		\
		register px_genlink_t* __z = (__z0);			\
		__z->link[0] = __y;					\
		__z->link[1] = __x;					\
		__x->link[0] = __z;					\
		__y->link[1] = __z;					\
	} while (0)

#define PxQCL_in_hi(__x0, __z0)						\
	do {								\
		register px_genlink_t* __x = (__x0);			\
		register px_genlink_t* __y = __x->link[1];		\
		register px_genlink_t* __z = (__z0);			\
		__z->link[0] = __x;					\
		__z->link[1] = __y;					\
		__y->link[0] = __z;					\
		__x->link[1] = __z;					\
	} while (0)

#define PxQCL_replace(__x0, __y0)					\
	do {								\
		register px_genlink_t* __x = (__x0);			\
		register px_genlink_t* __y = (__y0);			\
		if (__y != __y->link[0]) {				\
			*__x = *__y;					\
			__x->link[0]->link[1] = __x;			\
			__x->link[1]->link[0] = __x;			\
		} else {						\
			__x->link[0] = __x;				\
			__x->link[1] = __x;				\
		}							\
	} while (0)

PxAPI_FUNCEXT(void) pxqcl_init(px_genlink_t* root);
PxAPI_FUNCEXT(void) pxqcl_selflinked(px_genlink_t* root);
PxAPI_FUNCEXT(void) pxqcl_selfremove(px_genlink_t* root, bool order);
PxAPI_FUNCEXT(void) pxqcl_selfinsert_after(px_genlink_t* root, px_genlink_t* after, bool order);
PxAPI_FUNCEXT(void) pxqcl_selfinsert_before(px_genlink_t* root, px_genlink_t* before, bool order);
PxAPI_FUNCEXT(void) pxqcl_nullable(px_genlink_t* root);
PxAPI_FUNCEXT(void) pxqcl_merge(px_genlink_t* root, px_genlink_t* p, _u_int index);
PxAPI_FUNCEXT(void) pxqcl_merge_lo(px_genlink_t* root, px_genlink_t* p);
PxAPI_FUNCEXT(void) pxqcl_merge_hi(px_genlink_t* root, px_genlink_t* p);
PxAPI_FUNCEXT(void) pxqcl_extend(px_genlink_t* root, px_genlink_t* p, _u_int index);
PxAPI_FUNCEXT(void) pxqcl_extend_lo(px_genlink_t* root, px_genlink_t* p);
PxAPI_FUNCEXT(void) pxqcl_extend_hi(px_genlink_t* root, px_genlink_t* p);
PxAPI_FUNCEXT(void) pxqcl_in(px_genlink_t* root, px_genlink_t* p, _u_int index);
PxAPI_FUNCEXT(void) pxqcl_in_lo(px_genlink_t* root, px_genlink_t* p);
PxAPI_FUNCEXT(void) pxqcl_in_hi(px_genlink_t* root, px_genlink_t* p);
PxAPI_FUNCEXT(void) pxqcl_relplace(px_genlink_t* root, px_genlink_t* p);

#endif /* HEADER_PX_GENLINK */
