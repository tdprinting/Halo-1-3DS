/* Minimal stand-in for the Xbox XDK <xtl.h>: only the basic Windows types the engine's headers name. */
#ifndef HALO_COMPAT_XTL_H
#define HALO_COMPAT_XTL_H
#include <stdint.h>
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned long ULONG;
typedef void *HANDLE;
typedef void *LPVOID;
typedef const void *LPCVOID;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef union _LARGE_INTEGER { struct { DWORD LowPart; LONG HighPart; }; LONGLONG QuadPart; } LARGE_INTEGER;
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define MAX_PATH 260

/* exceptions / context (only used for crash handlers, which the port replaces) */
typedef struct _EXCEPTION_RECORD { DWORD ExceptionCode; } EXCEPTION_RECORD;
typedef struct _CONTEXT { DWORD Eip, Esp, Ebp; } CONTEXT;
typedef struct _EXCEPTION_POINTERS { EXCEPTION_RECORD *ExceptionRecord; CONTEXT *ContextRecord; } EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

/* memory */
#define PAGE_NOACCESS 0x01
#define PAGE_READONLY 0x02
#define PAGE_READWRITE 0x04

/* async io */
typedef struct _OVERLAPPED { DWORD Internal, InternalHigh, Offset, OffsetHigh; HANDLE hEvent; } OVERLAPPED, *LPOVERLAPPED;

/* Xbox live / network identity (layout per XDK) */
typedef struct { unsigned long s_addr; } XB_IN_ADDR;
typedef struct { XB_IN_ADDR ina; XB_IN_ADDR inaOnline; WORD wPortOnline; BYTE abEnet[6]; BYTE abOnline[20]; } XNADDR;
typedef struct { BYTE Signature[20]; } XCALCSIG_SIGNATURE;
#define MAX_GAMENAME 128
typedef struct { BYTE ab[8]; } XNKID;
typedef struct { BYTE ab[16]; } XNKEY;
typedef union _ULARGE_INTEGER { struct { DWORD LowPart; DWORD HighPart; }; ULONGLONG QuadPart; } ULARGE_INTEGER;
typedef struct _SYSTEMTIME { WORD wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds; } SYSTEMTIME;
typedef struct _MEMORYSTATUS { DWORD dwLength, dwMemoryLoad, dwTotalPhys, dwAvailPhys, dwTotalPageFile, dwAvailPageFile, dwTotalVirtual, dwAvailVirtual; } MEMORYSTATUS;
#define PAGE_WRITECOMBINE 0x400
enum { XC_LANGUAGE_UNKNOWN, XC_LANGUAGE_ENGLISH, XC_LANGUAGE_JAPANESE, XC_LANGUAGE_GERMAN, XC_LANGUAGE_FRENCH,
       XC_LANGUAGE_SPANISH, XC_LANGUAGE_ITALIAN, XC_LANGUAGE_KOREAN, XC_LANGUAGE_TCHINESE, XC_LANGUAGE_PORTUGUESE };


/* ---- D3D8 type names only (no API). The 3DS backend implements the engine's rasterizer_* seam instead. ---- */
typedef long HRESULT;
typedef unsigned long D3DRENDERSTATETYPE;
typedef unsigned long D3DTEXTURESTAGESTATETYPE;
typedef unsigned long D3DFORMAT;
typedef unsigned long D3DPRIMITIVETYPE;
typedef unsigned long D3DCOLOR;
typedef struct D3DDevice D3DDevice;
typedef struct D3DResource D3DResource;
typedef struct D3DBaseTexture D3DBaseTexture;
typedef struct D3DTexture D3DTexture;
typedef struct D3DSurface D3DSurface;
typedef struct D3DVertexBuffer D3DVertexBuffer;
typedef struct D3DIndexBuffer D3DIndexBuffer;

#endif
