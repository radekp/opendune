/* $Id$ */

/** @file src/os/endian.h Os-independent endian detection routines. */

#ifndef OS_ENDIAN_H
#define OS_ENDIAN_H

#if defined(_WIN32)
	#define __BYTE_ORDER __LITTLE_ENDIAN
	#if defined(_MSC_VER)
		#include <stdlib.h>
		#define __builtin_bswap32 _byteswap_ulong
	#endif /* _MSC_VER */
#elif defined(__APPLE__)
	#include <machine/endian.h>
#elif defined(__TINYC__)
	#include <endian.h>

	static uint32 __builtin_bswap32(uint32 x)
	{
		uint32 r = 0;
		r |= (x & 0xFF000000) >> 24;
		r |= (x & 0x00FF0000) >> 8;
		r |= (x & 0x0000FF00) << 8;
		r |= (x & 0x000000FF) << 24;
		return r;
	}
#elif defined(__i386) || defined(__x86_64)
	#include <byteswap.h>
	#define __builtin_bswap32(x) bswap_32(x)
#else
	#include <endian.h>

	#ifdef __arm__
		static __inline__ uint32 __builtin_bswap32(uint32 val)
		{
			__asm__ volatile (
				"eor     r3, %1, %1, ror #16\n\t"
				"bic     r3, r3, #0x00FF0000\n\t"
				"mov     %0, %1, ror #8\n\t"
				"eor     %0, %0, r3, lsr #8"
				: "=r" (val)
				: "0"(val)
				: "r3"
			);
			return val;
		}
	#endif
#endif /* _WIN32 */

#if __BYTE_ORDER == __LITTLE_ENDIAN
	#define HTOBE32(x) __builtin_bswap32(x)
	#define BETOH32(x) __builtin_bswap32(x)
	#define HTOBE16(x) (__builtin_bswap32(x) >> 16)
	#define BETOH16(x) (__builtin_bswap32(x) >> 16)
#else
	#define HTOBE32(x) (x)
	#define BETOH32(x) (x)
	#define HTOBE16(x) (x)
	#define BETOH16(x) (x)
#endif

#endif /* OS_ENDIAN_H */
