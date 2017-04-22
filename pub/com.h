#ifndef _IVM_PUB_COM_H_
#define _IVM_PUB_COM_H_

#define IVM_PRIVATE static
#define IVM_PUBLIC extern

#ifdef __GNUC__
	#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
		#define IVM_INLINE IVM_PRIVATE __inline__ __attribute__((always_inline))
	#else
		#define IVM_INLINE IVM_PRIVATE __inline__
	#endif
#elif defined(_MSC_VER)
	#define IVM_INLINE IVM_PRIVATE __forceinline
#elif (defined(__BORLANDC__) || defined(__WATCOMC__))
	#define IVM_INLINE IVM_PRIVATE __inline
#else
	#define IVM_INLINE IVM_PRIVATE inline
#endif

#define IVM_NOALIGN __attribute__((packed))

#define IVM_STRICT strict

#define IVM_ARRLEN(arr) (sizeof(arr) / sizeof(*arr))

#define IVM_INT_MAX(t) ((t)~((t)1 << (sizeof(t) * 8 - 1)))
#define IVM_UINT_MAX(t) (~(t)0)

#define IVM_PTR_DIFF(a, b, t) (((ivm_ptr_t)a - (ivm_ptr_t)b) / (ivm_long_t)sizeof(t))
#define IVM_PTR_SIZE sizeof(void *)

#if __GNUC__ >= 3 && 0
	#define IVM_LIKELY(x) __builtin_expect(!!(x), 1)
	#define IVM_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
	#define IVM_LIKELY(x) (x)
	#define IVM_UNLIKELY(x) (x)
#endif

#ifdef __cplusplus
	#define IVM_COM_HEADER extern "C" {
	#define IVM_COM_END }
#else
	#define IVM_COM_HEADER
	#define IVM_COM_END
#endif

#if defined(__linux__) || defined(__linux)
	#define IVM_OS_LINUX 1
#elif defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
	#define IVM_OS_WIN32 1
	#define IVM_OS_WIN64 1
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define IVM_OS_WIN32 1
#elif (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__) || defined(macintosh)
	#define IVM_OS_MAC 1
#else
	#error unsupported os type(remove this line if necessary)
#endif

#ifndef IVM_LIB_PATH
	#error library path not specified
#endif

#if defined(IVM_OS_WIN32)
	#define IVM_FILE_SEPARATOR '\\'
	#define IVM_FILE_SEPARATOR_S "\\"
#else
	#define IVM_FILE_SEPARATOR '/'
	#define IVM_FILE_SEPARATOR_S "/"
#endif

#endif
