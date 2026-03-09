/*
 * (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
 * e-mail: vysster@gmail.com
 */

#if !defined(HEADER_PX_TYPES)
#define HEADER_PX_TYPES

#include <sys/types.h>
#include <sys/stddef.h>

#if defined(__FreeBSD__)
#include <sys/limits.h>
#endif /* __FreeBSD__ */

#if defined(__linux__)
#include <stdint.h>
#endif /* __linux__ */

// _uX - unsigned value with size of pow(2,X) bits length
typedef uint8_t			_u3;
typedef uint16_t		_u4;
typedef uint32_t		_u5;	
typedef uint64_t		_u6;
typedef unsigned char		_ua;
typedef unsigned int		_uA;	
typedef unsigned long long	_uAA;
typedef uintptr_t		_u_ptr;
typedef unsigned char		_u_char;
typedef unsigned short		_u_short;
typedef unsigned int		_u_int;
typedef unsigned long		_u_long;
typedef unsigned long long	_u_long2;
// _sX - signed value with size of pow(2,X) bits length
typedef int8_t			_s3;
typedef int16_t			_s4;
typedef int32_t			_s5;
typedef int64_t			_s6;
typedef signed char		_sa;
typedef signed int		_sA;
typedef signed long long	_sAA;
typedef intptr_t		_s_ptr;
typedef signed char		_s_char;
typedef signed short		_s_short;
typedef signed int		_s_int;
typedef signed long		_s_long;
typedef signed long long	_s_long2;

typedef char			_da;
typedef int			_dA;
typedef long long		_dAA;
typedef char			_d_char;
typedef short			_d_short;
typedef int			_d_int;
typedef long			_d_long;
typedef long long		_d_long2;

#define px_sizeof_array(__bp)	(sizeof(__bp)/sizeof(*(__bp)))

#define _u(__tp)		_u##__tp
#define _s(__tp)		_s##__tp

#if !defined(px_c_cast)
#define px_c_cast(__struct,__field,__ptr) \
    ((__struct*)&(((char*)__ptr)[(char*)0 - (char*)&((__struct*)0)->__field]))
#endif /* c_cast */

#if !defined(px_c_re)
#define px_c_re(__struct,__field,__ptr)				\
	((__struct*)&(((_u_char*)__ptr)[(_u_char*)0 - (_u_char*)&((__struct*)0)->__field]))
#endif /* px_c_re */

#if !defined(px_c_re_define)
#define px_c_re_define(__struct,__field,__ptr,__asto)		\
	__struct* __asto = px_c_re(__struct,__field,__ptr)
#endif /* px_c_re_define */

#if !defined(px_c_re_assign)
#define px_c_re_assign(__var,__field,__ptr)			\
	__var = px_c_re(typeof(__var),__field,__ptr)
#endif /* px_c_re_assign */

// Определения, того, что раньше отдавалось в комментарии, только эти - еще
// и с типизированной проверкой пожелания наполнения констаннтного ряда.
//
//
// Чтобы не генерировались предупреждения, макросы предкомпиляции:
// - без знака по типизации;
// - со знаком по типизации.
// - без знака по переменной;
// - со знаком по переиенной;
#define px_type_is_unum(__def)	((~(__def)0) > (typeof(__x)0))
#define px_type_is_snum(__def)	(!px_type_is_unum(__def))
#define px_typeof_is_unum(__x)	((~typeof(__x)0) > (typeof(__x)0))
#define px_typeof_is_snum(__x)	(!px_typeof_is_unum(__x))
//
// Выбор начала отсчета, нуля, по типизации.
#define px_type_zero(__def)	((__def)0)
// Нижняя оценка половины значения, которое может принять на себя тип,
// легко расчитываемое.
#define px_type_median(__def)	(((__def)1)<<(px_bits_sizeof(__def) - 1))
// Определение 
#define px_type_biggest(__def)	(~((__def)0))
// Наименьшее 
#define px_type_lowest(__def)	((__def)0)


#define px_pow2(__bi)		(1U<<(__bi))

// !!! MACHDEPENDED !!!
#if 1
#define px_byte_pow2		(3) // where byte is octet
#endif

#define px_byte_bits		px_pow2(px_byte_pow2)
#define px_byte_bitmask		(px_byte_bits - 1)

#define pxbitle_test(__bp, __bi) \
	(((_u_char*)__bp)[(__bi)>>px_byte_pow2]&(1<<((__bi)&px_byte_bitmask)))
#define pxbitle_get(__bp, __bi)	 (pxbitle_test((__bp), (__bi)) != 0)
#define pxbitle_set0(__bp, __bi) \
	((_u_char*)__bp)[(__bi)>>px_byte_pow2] &= ~(1<<((__bi)&px_byte_bitmask))
#define pxbitle_set1(__bp, __bi) \
	((_u_char*)__bp)[(__bi)>>px_byte_pow2] |= 1<<((__bi)&px_byte_bitmask)

#define px_is_it_p2(__r)	(px_bits_reset_low(__r) == 0)
#define px_is_not_p2(__r)	(px_bits_reset_low(__r) != 0)

#define FLAGS_(__value,__var)	(((__var)&(__value)) == (__value))
#define ANYFLAG_(__value,__var)	(((__var)&(__value)) != 0)
#define FLAGSCOPY2_(__test,__value,__var) \
	__var = ((__test) ? ((__var)|(__value)) : ((__var)&~(__value)))

#define px_bits_sizeof(__r)	(sizeof(__r)*px_byte_bits)
#define px_bits_reset_low(__r)	(((__r) - 1)&(__r))
#define px_bits_rol(__r, __bi)	(((__r)<<(__bi))|((__r)>>(px_bits_sizeof(__r) - (__bi))))
#define px_bits_ror(__r, __bi)	(((__r)>>(__bi))|((__r)<<(px_bits_sizeof(__r) - (__bi))))

// Macros for find maximum. First argument have a priority.
#if !defined(choise_maxi)
#define choise_maxi(__x,__y)	(((__x) >= (__y)) ? (__x) : (__y))
#endif /* choise_maxi */

// Macros for find minumum. First argument have a priority.
#if !defined(choise_mini)
#define choise_mini(__x, __y)	(((__x) <= (__y)) ? (__x) : (__y))
#endif /* choise_mini */

/*
 * КОНСТАНТЫ УКАЗАТЕЛЕЙ.
 */
#if !defined(NULL)
#define NULL			((void*)0)
#endif /* NULL */

#if !defined(NONVALID)
#define NONVALID		((void*)&((_u_char*)0)[-1])
#endif /* NONVALID */



#if !defined(__cplusplus)
#if !defined(__bool_true_false_are_defined)
typedef _u_char bool;
#if !defined(true)
#define true ((bool)!0)
#endif /* true */
#if !defined(false)
#define false ((bool)!true)
#endif /* false */
#define __bool_true_false_are_defined
#endif /* __bool_true_false_are_defined */
#endif /* __cplusplus */

#if !defined(PxAPI_FUNCDEF)
#define PxAPI_FUNCDEF(__x)	__x
#endif /* PxAPI_FUNCDEF */

#if !defined(PxAPI_FUNCEXT)
#define PxAPI_FUNCEXT(__x)	extern __x
#endif /* PxAPI_FUNCEXT */

#if !defined(PxAPI_DATADEF)
#define PxAPI_DATADEF(__x)	__x
#endif /* PxAPI_DATADEF */

#if !defined(PxAPI_DATAEXT)
#define PxAPI_DATAEXT(__x)	extern __x
#endif /* PxAPI_DATAEXT */

#if !defined(PxAPI_STATIC)
#define PxAPI_STATIC(__x)	static __x
#endif /* PxAPI_STATIC */

#endif /* HEADER_PX_TYPES */
