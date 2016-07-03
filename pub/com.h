#ifndef _IVM_PUB_COM_H_
#define _IVM_PUB_COM_H_

#define IVM_PRIVATE static
#define IVM_PUBLIC extern

#define IVM_INLINE inline __attribute__((always_inline))
#define IVM_NOALIGN __attribute__((packed))

#define IVM_STRICT strict

#define IVM_ARRLEN(arr) (sizeof(arr) / sizeof(*arr))

#define IVM_INT_MAX(t) ((t)~((t)1 << (sizeof(t) * 8 - 1)))
#define IVM_UINT_MAX(t) (~(t)0)

#ifdef __cplusplus
	#define IVM_COM_HEADER extern "C" {
	#define IVM_COM_END }
#else
	#define IVM_COM_HEADER
	#define IVM_COM_END
#endif

#if defined(__linux__) || defined(__linux)
	#define IVM_OS_LINUX
#elif defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
	#define IVM_OS_WIN32
	#define IVM_OS_WIN64
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define IVM_OS_WIN32
#endif

#endif
