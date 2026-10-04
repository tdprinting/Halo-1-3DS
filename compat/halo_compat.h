/* Force-included (-include) when compiling upstream Halo sources with GCC/newlib instead of MSVC/XDK.
 * Does not modify upstream; source-level changes live in patches/. */
#ifndef HALO_COMPAT_H
#define HALO_COMPAT_H

#ifndef _MSC_VER

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <wchar.h>

/* MSVC keywords */
#define __int64 long long
#define __int32 int
#define __int16 short
#define __int8 char
#define __cdecl
#define __stdcall
#undef __fastcall
#define __fastcall
#define __forceinline inline
#define __inline inline
#define __declspec(x)

/* cseries.h defines LONG_MAX etc. as enum constants, so <limits.h> must not be included. */
#ifndef SCHAR_MAX
#define SCHAR_MAX 127
#endif

#include "halo_struct_forwards.h"

/* Engine functions whose names collide with libc (declared in cseries.h / real_math.h). */
#define strnlen halo_strnlen
#define random halo_random

/* MSVC's wide CRT differs from newlib's (and newlib's wcs* assume 32-bit wchar_t, but the game uses
 * 16-bit -fshort-wchar). The game links libs/libcmt's wcs* instead; rename the ones whose prototypes differ. */
#define wcstok halo_wcstok
#define vswprintf halo_vswprintf
#define swprintf halo_swprintf
#define _snwprintf halo_snwprintf
#define _vsnwprintf halo_vsnwprintf
wchar_t *halo_wcstok(wchar_t *string, const wchar_t *delimiters);
int halo_vswprintf(wchar_t *buffer, const wchar_t *format, va_list arguments);
int halo_swprintf(wchar_t *buffer, const wchar_t *format, ...);
int halo_snwprintf(wchar_t *buffer, size_t count, const wchar_t *format, ...);
int halo_vsnwprintf(wchar_t *buffer, size_t count, const wchar_t *format, va_list arguments);

/* hs.c defines its own isspace(); newlib's ctype macro would collide. */
#undef isspace
#define isspace halo_isspace

/* x87 precision control has no ARM equivalent */
#define CW_DEFAULT 0
#define _control87(a, b) ((void)0)

/* MSVC CRT names */
#define _stricmp strcasecmp
#define stricmp strcasecmp
#define _strnicmp strncasecmp
#define strnicmp strncasecmp
#define _snprintf snprintf
#define _vsnprintf vsnprintf
#define _strdup strdup

#endif /* !_MSC_VER */
#endif
