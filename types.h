#ifndef _INCLUDE_TYPES_H_
#define _INCLUDE_TYPES_H_

#if defined(_MSC_VER)

typedef char int8;
typedef short int16;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;
typedef long long int int64;
typedef unsigned long long int uint64;

#else

#include <stdint.h>

typedef char int8;
typedef short int16;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef long long int int64;
typedef unsigned long long int uint64;

#endif

typedef float FLOAT_T;

#include <stdlib.h>
#include <string.h>
#define MEM_ALLOC_FUNC malloc
#define MEM_CALLOC_FUNC calloc
#define MEM_REALLOC_FUNC realloc
#define MEM_STRDUP_FUNC strdup
#define MEM_FREE_FUNC free

#ifdef EXTERN
# undef EXTERN
#endif

#ifdef _MSC_VER
# ifdef __cplusplus
#  define EXTERN extern "C" __declspec(dllexport)
# else
#  define EXTERN extern __declspec(dllexport)
# endif
#else
# define EXTERN extern
#endif

#ifndef FALSE
# define FALSE (0)
# define TRUE !(FALSE)
#endif

#ifndef M_PI
# define M_PI 3.1415926535897932384626433832795
#endif

#define DIV_PIXEL ((FLOAT_T)0.00392157f)

typedef struct _GAME_DATA GAME_DATA;

#endif	// #ifndef _INCLUDE_TYPES_H_
