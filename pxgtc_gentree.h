/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(HEADER_PXGTC_GENTREE)
#define HEADER_PXGTC_GENTREE

#include "px_types.h"
#include "px_genlink.h"

typedef px_genlink_t* (pxgtc_select_t)(px_genlink_t* p);
typedef void (pxgtc2qcl_t)(px_genlink_t*** path, px_genlink_t** pp, px_genlink_t* qcl);
typedef px_genlink_t*** (pxgtc_initial_t)(px_genlink_t*** path, px_genlink_t** pp);
typedef px_genlink_t*** (pxgtc_iterator_t)(px_genlink_t*** path, px_genlink_t*** ppp);

PxAPI_FUNCEXT(pxgtc2qcl_t) pxgtc2qcl_next;
PxAPI_FUNCEXT(pxgtc2qcl_t) pxgtc2qcl_prev;
PxAPI_FUNCEXT(pxgtc_select_t) pxgtc_selectmin;
PxAPI_FUNCEXT(pxgtc_select_t) pxgtc_selectmax;
PxAPI_FUNCEXT(pxgtc_initial_t) pxgtc_pathmin;
PxAPI_FUNCEXT(pxgtc_initial_t) pxgtc_pathmax;
PxAPI_FUNCEXT(pxgtc_iterator_t) pxgtc_vprev;
PxAPI_FUNCEXT(pxgtc_iterator_t) pxgtc_vnext;
PxAPI_FUNCEXT(pxgtc_iterator_t) pxgtc_wprev;
PxAPI_FUNCEXT(pxgtc_iterator_t) pxgtc_wnext;
PxAPI_FUNCEXT(pxgtc_iterator_t) pxgtc_xprev;
PxAPI_FUNCEXT(pxgtc_iterator_t) pxgtc_xnext;
PxAPI_FUNCEXT(pxgtc_iterator_t) pxgtc_zprev;
PxAPI_FUNCEXT(pxgtc_iterator_t) pxgtc_znext;

#endif /* HEADER_PXGTC_GENTREE */
