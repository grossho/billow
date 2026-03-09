/*
 * (c) Vladimir Yu. Stepanov 2006-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(HEADER_BW_TREEDICT)
#define HEADER_BW_TREEDICT

#include <Python.h>
#include "px_types.h"
#include "px_genlink.h"
#include "pxgtc_balance.h"
#include "pxgtc_gentree.h"

typedef struct __bw_treedict_node_t {
	px_genlink_t n_link;
	PyObject* n_key;
	PyObject* n_value;
	union {
		struct {
			_u5 n_state_avl:2;
			_u5 n_rank_avl:30;
		};
		struct {
			_u5 n_state_rb:1;
			_u5 n_rank_rb:31;
		};
	};
} bw_treedict_node_t;

typedef struct __bw_treedict_t {
	PyObject_HEAD

	_s5 td_length;
	_s5 td_lockcount;
	px_genlink_t* td_root;
	PyObject* td_compare;
	pxgtc_algo_t* td_algo;
	px_genlink_t** td_path[PxGTC_MAXPATH];
} bw_treedict_t;

typedef PyObject* (bw_treedict_getitem_t)(bw_treedict_node_t* n);

typedef struct __bw_treedict_iter_t {
	PyObject_HEAD

	bw_treedict_t* it_td;
	pxgtc_iterator_t* it_step;
	bw_treedict_getitem_t* it_getitem;
	px_genlink_t*** it_k;
	px_genlink_t** it_path[PxGTC_MAXPATH];
} bw_treedict_iter_t;

extern PyObject* bw_cmp_buffer(PyObject* self, PyObject* args);
#if PY_VERSION_HEX >= 0x03000000
extern PyObject* bw_cmp_long(PyObject* self, PyObject* args);
#endif /* PY_VERSION_HEX >= 0x03000000 */

extern PyTypeObject bw_treedict_Type;
extern PyTypeObject bw_treedict_iter_Type;

#endif /* HEADER_BW_TREEDICT */
