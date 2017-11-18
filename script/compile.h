#ifndef _INCLUCED_COMPILE_H_
#define _INCLUCED_COMPILE_H_

#include <stdarg.h>
#include "../utils.h"
#include "../memory_pool.h"

typedef struct _SCRIPT_COMPILER
{
	POINTER_ARRAY *abstract_syntax_tree;
	MEMORY_POOL *memory_pool;
	STRING_HASH_TABLE *reserved;
	STRING_HASH_TABLE *functions;
	STRING_HASH_TABLE *global;
} SCRIPT_COMPILER;

#endif	// #ifndef _INCLUCED_COMPILE_H_
