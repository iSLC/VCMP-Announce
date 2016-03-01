#ifndef _LIBRARY_BASE_HPP_
#define _LIBRARY_BASE_HPP_

// ------------------------------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>

/* ------------------------------------------------------------------------------------------------
 * ARCHITECTURE IDENTIFIERS
*/

#define SMOD_ARCH_ID_UNKNOWN     0
#define SMOD_ARCH_ID_32_BIT      1
#define SMOD_ARCH_ID_64_BIT      2

/* ------------------------------------------------------------------------------------------------
 * PLATFORM IDENTIFIERS
*/

#define SMOD_PLAT_ID_UNKNOWN     0
#define SMOD_PLAT_ID_WINDOWS     1
#define SMOD_PLAT_ID_LINUX       2
#define SMOD_PLAT_ID_MACOS       3
#define SMOD_PLAT_ID_UNIX        4

/* ------------------------------------------------------------------------------------------------
 * OS IDENTIFICATION
*/

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN) || defined(__WIN__) || defined(__WINDOWS__)
    // Windows x32
    #define SMOD_OS_WINDOWS
    #define SMOD_OS_32
    #define SMOD_OS_WINDOWS32
    #define SMOD_ARCHITECTURE 1
    #define SMOD_PLATFORM 1
    // Threads
    #if !defined(SMOD_THREAD_WIN32) && !defined(SMOD_THREAD_POSIX)
        #define SMOD_THREAD_WIN32
    #endif
#elif defined(_WIN64) || defined(__WIN64__)
    // Windows x64
    #define SMOD_OS_WINDOWS
    #define SMOD_OS_64
    #define SMOD_OS_WINDOWS64
    #define SMOD_ARCHITECTURE 2
    #define SMOD_PLATFORM 1
    // Threads
    #if !defined(SMOD_THREAD_WIN32) && !defined(SMOD_THREAD_POSIX)
        #define SMOD_THREAD_WIN32
    #endif
#elif defined(linux) || defined(__linux) || defined(__linux__)
    // Linux
    #define SMOD_OS_LINUX
    #if __GNUC__
        #if __x86_64__ || __ppc64__
            #define SMOD_OS_64
            #define SMOD_OS_LINUX64
            #define SMOD_ARCHITECTURE 2
            #define SMOD_PLATFORM 2
        #else
            #define SMOD_OS_32
            #define SMOD_OS_LINUX32
            #define SMOD_ARCHITECTURE 1
            #define SMOD_PLATFORM 2
        #endif
        // Threads
        #if !defined(SMOD_THREAD_WIN32) && !defined(SMOD_THREAD_POSIX)
            #define SMOD_THREAD_POSIX
        #endif
    #endif
#elif defined(__APPLE__) || defined(__MACH__) || defined(MACOSX) || defined(macintosh) || defined(Macintosh)
    // MacOS
    #define SMOD_OS_MACOS
    #if __GNUC__
        #if __x86_64__ || __ppc64__
            #define SMOD_OS_64
            #define SMOD_OS_MACOS64
            #define SMOD_ARCHITECTURE 2
            #define SMOD_PLATFORM 3
        #else
            #define SMOD_OS_32
            #define SMOD_OS_MACOS32
            #define SMOD_ARCHITECTURE 1
            #define SMOD_PLATFORM 3
        #endif
        // Threads
        #if !defined(SMOD_THREAD_WIN32) && !defined(SMOD_THREAD_POSIX)
            #define SMOD_THREAD_POSIX
        #endif
    #endif
#elif defined(__unix) || defined(__unix__)
    // Unix
    #define SMOD_OS_UNIX
    #if __GNUC__
        #if __x86_64__ || __ppc64__
            #define SMOD_OS_64
            #define SMOD_OS_UNIX64
            #define SMOD_ARCHITECTURE 2
            #define SMOD_PLATFORM 4
        #else
            #define SMOD_OS_32
            #define SMOD_OS_UNIX32
            #define SMOD_ARCHITECTURE 1
            #define SMOD_PLATFORM 4
        #endif
        // Threads
        #if !defined(SMOD_THREAD_WIN32) && !defined(SMOD_THREAD_POSIX)
            #define SMOD_THREAD_POSIX
        #endif
    #endif
#else
    // Unsupported system
    #error This operating system is not supported by the Announcer
#endif

#ifndef SMOD_ARCHITECTURE
     #define SMOD_ARCHITECTURE 0
#endif

#ifndef SMOD_PLATFORM
     #define SMOD_PLATFORM 0
#endif

// ------------------------------------------------------------------------------------------------
namespace SMod {

// ------------------------------------------------------------------------------------------------
/**< 8 bits integer types */
typedef char                        Int8, I8;
typedef unsigned char               Uint8, U8;

// ------------------------------------------------------------------------------------------------
/**< 16 bits integer types */
typedef short                       Int16, I16;
typedef unsigned short              Uint16, U16;

// ------------------------------------------------------------------------------------------------
/**< 32 bits integer types */
typedef int                         Int32, I32;
typedef unsigned int                Uint32, U32;

// ------------------------------------------------------------------------------------------------
/**< 64 bits integer types */
#if defined(_MSMOD_VER)
    typedef __int64                 Int64, I64;
    typedef unsigned __int64        Uint64, U64;
#else
    typedef long long               Int64, I64;
    typedef unsigned long long      Uint64, U64;
#endif

// ------------------------------------------------------------------------------------------------
/**< integer type */
#ifdef SMOD_LONG
    typedef Int64                   Int, Integer;
    typedef Uint64                  Uint, Uinteger, UnisgnedInteger;
#else
    typedef Int32                   Int, Integer;
    typedef Uint32                  Uint, Uinteger, UnisgnedInteger;
#endif

// ------------------------------------------------------------------------------------------------
/**< long integer type */
typedef long                        LongI;
typedef unsigned long               Ulong;

// ------------------------------------------------------------------------------------------------
/**< 32 bits float types */
typedef float                       Float32;

/**< 64 bits float types */
typedef double                      Float64;

// ------------------------------------------------------------------------------------------------
/**< float type */
#ifdef SQ_DOUBLE
    typedef double                  Float, Real;
else
    typedef float                   Float, Real;
#endif

// ------------------------------------------------------------------------------------------------
/**< boolean type */
typedef unsigned char               Boolean;

/**< character type */
typedef bool                        BoolT;

// ------------------------------------------------------------------------------------------------
/**< character type */
typedef char                        CharT;
typedef wchar_t                     WcharT;
typedef unsigned char               UcharT;

// ------------------------------------------------------------------------------------------------
/**< user type */
typedef void *                      VoidP;

// ------------------------------------------------------------------------------------------------
/**< size type */
typedef size_t                      SizeT;

// ------------------------------------------------------------------------------------------------
/* C style ASCII string */
typedef CharT *                     CStr;
typedef const CharT *               CCStr;

// ------------------------------------------------------------------------------------------------
/* C style Wide string */
typedef WcharT *                    CWStr;
typedef const WcharT *              CCWStr;

} // Namespace:: SMod

/* ------------------------------------------------------------------------------------------------
 * OS SPECIFFIC OPTIONS
*/

#if defined(SMOD_OS_WINDOWS)
    #define SMOD_DIRSEP_CHAR    '\\'
    #define SMOD_DIRSEP_STR     "\\"
#else
    #define SMOD_DIRSEP_CHAR    '/'
    #define SMOD_DIRSEP_STR     "/"
#endif

/* ------------------------------------------------------------------------------------------------
 * SYMBOL EXPORTING
*/

#if defined(_MSMOD_VER)
    #define SMOD_EXPORT         __declspec(dllexport)
    #define SMOD_IMPORT         __declspec(dllimport)
#elif defined(__GNUC__)
    #define SMOD_EXPORT         __declspec(dllexport)
    #define SMOD_IMPORT         __declspec(dllimport)
#endif

#if defined(__cplusplus)
    #define SMOD_EXTERN_C       extern "C"
#else
    #define SMOD_EXTERN_C       /* */
#endif

#if defined(_MSMOD_VER)
    #define SMOD_API_EXPORT     extern "C" __declspec(dllexport)
#elif defined(__GNUC__)
    #define SMOD_API_EXPORT     extern "C"
#endif

/* ------------------------------------------------------------------------------------------------
 * CALLING CONVENTIONS
*/

#if defined(_MSMOD_VER)
    #define SMOD_STDCALL        __stdcall
    #define SMOD_CDECL          __cdecl
    #define SMOD_FASTCALL       __fastcall
#elif defined(__GNUC__)
    #define SMOD_STDCALL        __attribute__((stdcall))
    #define SMOD_CDECL          /* */
    #define SMOD_FASTCALL       __attribute__((fastcall))
#endif

/* ------------------------------------------------------------------------------------------------
 * FUNCTION INLINING
*/

#if defined(_MSMOD_VER)
    #define SMOD_FORCEINLINE    __forceinline
#elif defined(__GNUC__)
    #define SMOD_FORCEINLINE    inline
#endif

/* ------------------------------------------------------------------------------------------------
 * VARIOUS DEFINES
*/

#define SMOD_DECL_UNUSED_VAR(t, n, v) t n = v; (void)(n)
#define SMOD_UNUSED_VAR(n) (void)(n)

/* ------------------------------------------------------------------------------------------------
 * GENERAL RESPONSES
*/

#define SMOD_SUCCESS   1
#define SMOD_FAILURE   0
#define SMOD_UNKNOWN   -1
#define SMOD_TRUE      1
#define SMOD_FALSE     0
#define SMOD_NULL      NULL
#define SMOD_BLANK     0

#endif // _LIBRARY_BASE_HPP_
