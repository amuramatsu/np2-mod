#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<stddef.h>

#define	MACOS
#if defined(__i386__)||defined(__x86_64__)
#define	BYTESEX_LITTLE
#ifdef __i386__
#define	NP2_CPU_ARCH_IA32
#endif
#ifdef __x86_64__
#define	NP2_CPU_ARCH_AMD64
#endif
#else
#define	BYTESEX_BIG
#endif
#define	OSLANG_SJIS
#define	OSLINEBREAK_CR

#if defined(NP2_CPU_ARCH_IA32)
#define	GCC_CPU_ARCH_IA32
#endif
#if defined(NP2_CPU_ARCH_AMD64)
#define	GCC_CPU_ARCH_AMD64
#endif

typedef signed int		SINT;
typedef unsigned int	UINT;

typedef signed char		SINT8;
typedef unsigned char	UINT8;

typedef signed short	SINT16;
typedef unsigned short	UINT16;

typedef signed int		SINT32;
typedef unsigned int	UINT32;

typedef Boolean			BOOL;
typedef signed char		TCHAR;
typedef signed char		CHAR;
typedef unsigned char	BYTE;


#define	INLINE		inline

#define	MAX_PATH	260

#define	ZeroMemory(a, b)		memset((a),  0 , (b))
#define	FillMemory(a, b, c)		memset((a), (c), (b))
#define	CopyMemory(a, b, c)		memcpy((a), (b), (c))

#define	max(a, b)				(((a)>(b))?(a):(b))
#define	min(a, b)				(((a)<(b))?(a):(b))

#define	BRESULT				UINT8
#define	OEMCHAR				char
#define	OEMTEXT(string)		string
#define	OEMSPRINTF			sprintf
#define	OEMSTRLEN			strlen

#include	"common.h"
#include	"macossub.h"
#include	"milstr.h"
#include	"_memory.h"
#include	"rect.h"
#include	"lstarray.h"
#include	"trace.h"

#define	GETTICK()			macos_gettick()
#define	SPRINTF				sprintf
#define	STRLEN				strlen
#define	__ASSERT(s)

#define	VERMOUTH_LIB
// #define SOUND_CRITICAL

#if defined(OSLANG_SJIS)
#define	SUPPORT_SJIS
#elif defined(OSLANG_UTF8)
#define	SUPPORT_UTF8
#else
#define	SUPPORT_ANK
#endif

#undef	SUPPORT_8BPP
#define	SUPPORT_16BPP
#undef	SUPPORT_24BPP
#define	SUPPORT_32BPP
#undef	SUPPORT_NORMALDISP
#if !(defined(__i386__)||defined(__x86_64__))
#define	MEMOPTIMIZE		1
#endif

#if defined(CPUCORE_IA32)
#include "trace.h"
#define	msgbox(title, msg) \
 do { TRACEOUT(("%s", title)); TRACEOUT(("%s", msg)) } while (/*CONSTCOND*/0)
#if !defined(DISABLE_PC9821)
#define	SUPPORT_CRT31KHZ
#define	SUPPORT_PC9821
#endif
#define	SUPPORT_IDEIO
#define	IA32_PAGING_EACHSIZE
#else
#define	SUPPORT_CRT15KHZ
#endif
#define	SUPPORT_S98
#define	SUPPORT_SWSEEKSND
#define	SUPPORT_HOSTDRV
#define	SUPPORT_SASI
#define	SUPPORT_SCSI
#define SUPPORT_KEYDISP
#define	SUPPORT_JOYSTICK
#define SUPPORT_SOFTKBD	0

#define	SUPPORT_RESUME
#define	SUPPORT_STATSAVE
#define	SOUNDRESERVE	80

#if defined(NP2_CPU_ARCH_IA32)
#undef	MEMOPTIMIZE
#define LOADINTELDWORD(a)	(*((const UINT32 *)(a)))
#define LOADINTELWORD(a)	(*((const UINT16 *)(a)))
#define STOREINTELDWORD(a, b)	*(UINT32 *)(a) = (b)
#define STOREINTELWORD(a, b)	*(UINT16 *)(a) = (b)
#if !defined(DEBUG) && !defined(NP2_CPU_ARCH_AMD64)
#define	FASTCALL	__attribute__((regparm(2)))
#endif	/* !DEBUG && !NP2_CPU_ARCH_AMD64 */
#elif defined(arm) || defined (__arm__)
#define	MEMOPTIMIZE	2
#define	REG8		UINT
#define	REG16		UINT
#define	OPNGENARM
#else
#define	MEMOPTIMIZE	1
#endif

typedef SInt64		SINT64;
typedef UInt64		UINT64;
#ifndef FASTCALL
#define FASTCALL
#endif
#define CPUCALL		FASTCALL
#define MEMCALL		FASTCALL
#define DMACCALL	FASTCALL
#define IOOUTCALL	FASTCALL
#define IOINPCALL	FASTCALL
#define SOUNDCALL	FASTCALL
#define VRAMCALL	FASTCALL
#define SCRNCALL	FASTCALL
#define VERMOUTHCL	FASTCALL
//#define	sigjmp_buf				jmp_buf
//#define	sigsetjmp(env, mask)	setjmp(env)
//#define	siglongjmp(env, val)	longjmp(env, val)
