#ifndef __NP2_COMPILER_H__
#define __NP2_COMPILER_H__
#include "compiler_base.h"
#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<stddef.h>

#define	MACOS
#ifdef __BIG_ENDIAN__
#define	BYTESEX_BIG
#else
#define	BYTESEX_LITTLE
#endif
#define	OSLANG_UTF8
#define SUPPORT_SJIS
#define	OSLINEBREAK_LF

/* archtecture */
#if defined(amd64) || defined(__AMD64__) || defined(__amd64__) || \
	defined(x86_64) || defined(__x86_64__) || defined(__X86_64__) || \
	defined(__aarch64__) || defined(_WIN64) || defined(_M_X64) || \
	defined(__LP64__) || defined(__LLP64__) || defined(__LLP64__)
#define	NP2_CPU_64BIT
#endif
#if defined(i386) || defined(__i386__) || defined(__arm__) || \
	defined(_WIN32) || defined(_M_IX86) || \
	defined(NP2_CPU_ARCH_AMD64)
#define	NP2_CPU_32BIT
#endif

#if defined(amd64) || defined(__AMD64__) || defined(__amd64__) || \
	defined(x86_64) || defined(__x86_64__) || defined(__X86_64__)
#define	NP2_CPU_ARCH_AMD64
#endif
#if defined(i386) || defined(__i386__) || defined(NP2_CPU_ARCH_AMD64)
#define	NP2_CPU_ARCH_IA32
#endif
#if defined(__GNUC__)
#if defined(NP2_CPU_ARCH_IA32)
#define	GCC_CPU_ARCH_IA32
#endif
#if defined(NP2_CPU_ARCH_AMD64)
#define	GCC_CPU_ARCH_AMD64
#endif
#endif

#define _T(string) string
#define _tcscpy    OEMSTRCPY
#define	_tcsicmp   milstr_cmp
#define	_tcsnicmp  strncasecmp

// --> Windowsnize
// calling convention
#undef  CDECL
#undef  STDCALL
#undef  CLRCALL
#undef  FASTCALL
#undef  VECTORCALL
#if defined(__cpluscplus)
#undef  THISCALL
#endif

#if defined(_MSC_VER) && defined(_M_IX86) && !defined(LR_VS2017)
#define CDECL      __cdecl
#define STDCALL    __stdcall
#define FASTCALL   __fastcall
#define SAFECALL   __safecall
#define CLRCALL    __clrcall
#define VECTORCALL __vectorcall
#if defined(__cpluscplus)
#define THISCALL   __thiscall
#endif
#elif defined(__GNUC__) && defined(__i386__) && !defined(__ANDROID__) && !defined(EMSCRIPTEN)
#define CDECL      __attribute__ ((cdecl))
#define STDCALL    __attribute__ ((stdcall))
#define FASTCALL   __attribute__ ((fastcall))
#define CLRCALL
#define VECTORCALL __attribute__ ((interrupt))
#if defined(__cpluscplus)
#define THISCALL   __attribute__ ((thiscall))
#endif
#else
#define CDECL
#define STDCALL
#define FASTCALL
#define SAFECALL
#define CLRCALL
#define VECTORCALL
#if defined(__cpluscplus)
#define THISCALL
#endif
#endif

#define	CPUCALL    FASTCALL
#define	MEMCALL    FASTCALL
#define	DMACCALL   FASTCALL
#define	IOOUTCALL  FASTCALL
#define	IOINPCALL  FASTCALL
#define	SOUNDCALL  FASTCALL
#define	VRAMCALL   FASTCALL
#define	SCRNCALL   FASTCALL
#define	VERMOUTHCL FASTCALL
#define	PARTSCALL  FASTCALL

// inline
#if !defined(DEBUG) || !defined(_DEBUG)
#if !defined(INLINE)
#if defined(__GNUC__)
#define INLINE __inline__ __attribute__((always_inline))
#else
#define INLINE
#endif
#endif
#else
#undef  INLINE
#define INLINE
#endif


#define	max(a, b)				(((a)>(b))?(a):(b))
#define	min(a, b)				(((a)<(b))?(a):(b))

#define	OEMCHAR				char
#define	OEMTEXT(string)		string
#define	OEMSPRINTF			sprintf
#define OEMSNPRINTF			snprintf
#define	OEMSTRLEN			strlen



#include	"common.h"
#include	"macossub.h"
#include	"milstr.h"
#include	"_memory.h"
#include	"rect.h"
#include	"lstarray.h"
#include	"trace.h"

#define	msgbox(title, msg)

#define	GETTICK()			macos_gettick()
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

// #define	SUPPORT_8BPP
#ifdef NP2GCC
#define	SUPPORT_16BPP
#endif
// #define	SUPPORT_24BPP
#define	SUPPORT_32BPP
// #define SUPPORT_NORMALDISP
#define	MEMOPTIMIZE		1

#if defined(CPUCORE_IA32)
#define	SUPPORT_CRT31KHZ
#define	SUPPORT_PC9821
#define	IA32_PAGING_EACHSIZE
#endif
#define	SUPPORT_CRT15KHZ
#define	SUPPORT_S98
#define	SUPPORT_SWSEEKSND
#define	SUPPORT_HOSTDRV
#define	SUPPORT_SASI
#define	SUPPORT_SCSI
#define SUPPORT_KEYDISP
#define SUPPORT_SOFTKBD	0

#define	USE_RESUME
#define	SOUNDRESERVE	80

#if defined(CPUCORE_IA32)
typedef SInt64			SINT64;
typedef UInt64			UINT64;
#define FASTCALL
#define CPUCALL
#define MEMCALL
#define	SUPPORT_PC9821
#define	SUPPORT_CRT31KHZ
#define IA32_PAGING_EACHSIZE
#define	sigjmp_buf				jmp_buf
#define	sigsetjmp(env, mask)	setjmp(env)
#define	siglongjmp(env, val)	longjmp(env, val)
#define	msgbox(title, msg)		TRACEOUT(("%s", title)); \
								TRACEOUT(("%s", msg))
#endif

#endif
