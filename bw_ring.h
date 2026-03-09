/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(HEADER_BW_RING)
#define HEADER_BW_RING

#include <Python.h>

#include "px_types.h"
#include "px_genlink.h"
#include "billow.h"
#include "pycompat.h"

typedef int32_t bwsize_t;

typedef struct __bw_ring_state_t {
	_s5 state_elements;
	// Количество открытых итераторов. Последний вызывает деструкцию
	// всего объекта.
	_s5 state_cursors;
	// Объект для координации действий итераторов.
	PyObject* state_anchor;
} bw_ring_state_t;



typedef struct __bw_ring_node_t {
	// Двусвязанный чиклический список из узлов и базовых узлов, для
	// которых выполняется no_item == NULL.
	// Хранимое значение (для псево-узла равно NULL).
	px_genlink_t n_p;
	PyObject* n_body;
} bw_ring_node_t;

typedef struct __bw_ring_head_t {
	// Узел.
	bw_ring_node_t hd_n;
	// Если обнулен, то структура является дополнительной к курсорному
	// объекту.
	bw_ring_state_t* hd_state;
} bw_ring_head_t;

typedef struct __bw_ring_t {
	PyObject_HEAD
	// Указатель на текущую позицию курсора. Может указывать на ring_node.
	// Если обнулен - next возвращает NULL.

	// Базовый (стартовый и завершающий) элемент. Последнее поле
	// используется как флаг.
	bw_ring_head_t ring_head;
	bw_ring_head_t ring_stop;
	// Количество курсоров, указывающих на заданный узел. Для
	// псевдо-узла роль счетчика играет один младший бит,
	// поскольку на него может ссылаться только сам объект, для
	// которого он является базовым. Остальные биты псевдо-узла
	// используются для хранения флагов.
	bool ring_order;
} bw_ring_t;

extern PyTypeObject bw_ring_Type;

#endif /* HEADER_BW_RING */
