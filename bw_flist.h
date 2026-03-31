/*
 * (c) Vladimir Yu. Stepanov 2010-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(HEADER_BILLOW_FLIST)
#define HEADER_BILLOW_FLIST

#include <sys/types.h>

#include <Python.h>
#include "px_types.h"
#include "pycompat.h"

typedef struct __bw_flist {
	PyObject_HEAD

	// Корневой элемент.
	void** flist_rootptr;

	// Размер кучи. Указывает на последний элемент, тем и отличается
	// от стандартных типов PyObject_VAR_HEAD.
	_s6 flist_cpos;
	// Закешированный объем указателей, заданных через указатель на
	// последний кешированный элемент.
	_s6 flist_precached_cpos;
	// Блокирование на чтение (положительное число), когда читающие
	// процессы ыозможны во множестве, или на запись какой-то процесс
	// заблокировал и чтение (отрицательное число). Запрещает вносить
	// изменения в структуру мета-индекса.
	_s5 flist_locked;
	// Первичный сдвиг для получения индекса в корневом элементе. То есть
	// для индекса с прямой адресацией это будет 0.
	_s3 flist_shiftlevel;
	// Степень двойки для строки индекса. По нему расчитывается
	// flist_linesize.
	_u3 flist_linesize_p2;
	// Заблокирован замком на чтение.
	_u3 flist_frozenlock;
	_u3 flist_unused;
} bw_flist;

struct __bw_flistiter;

typedef PyObject* (bw_flistiter_funcnext)(struct __bw_flistiter* iter);

typedef struct __bw_flistiter {
	PyObject_HEAD

	// Ссылка на итерируемый объект.
	bw_flist* it_flist;
	void** it_lastptr;
	Py_ssize_t *it_lesser;
	Py_ssize_t *it_greater;
	// Шаг итерации.
	Py_ssize_t it_step;
	// Последний индекс для подгрузки нового прекешированного значения.
	Py_ssize_t it_lastindex;
	// Прекешированное значение.
	Py_ssize_t it_cpos;
	// Значение используется неявно в it_lesser или it_greater.
	Py_ssize_t it_lastpos;
} bw_flistiter;

#define PyBILLOW_FLIST_LINEMIN 2
#define PyBILLOW_FLIST_LINEMAX 65536
//#define PyBILLOW_FLIST_LINEDEFAULT 128
//#define PyBILLOW_FLIST_LINEDEFAULT 1024
#define PyBILLOW_FLIST_LINEDEFAULT 4

extern PyTypeObject bw_pair_Type;
extern PyTypeObject bw_pair_iter_Type;

#endif /* HEADER_BILLOW_FLIST */
