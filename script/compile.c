#include <string.h>
#include "compile.h"
#include "token.h"
#include "parser.h"
#include "operand.h"

#define CODE_BLOCK_SIZE 4096

#if 0
void InitializeScriptCompiler(
	SCRIPT_COMPILER* compiler,
	POINTER_ARRAY* abstract_syntax_tree,
	STRING_HASH_TABLE* reserved,
	STRING_HASH_TABLE* functions,
	MEMORY_POOL* memory_pool
)
{
	(void)memset(compiler, 0, sizeof(*compiler));

	compiler->abstract_syntax_tree = abstract_syntax_tree;
	compiler->reserved = reserved;
	compiler->functions = functions;
	compiler->memory_pool;
}

static size_t AbstractSyntaxTreeToByteCodeLocal(
	SCRIPT_COMPILER* compiler,
	uint8** code,
	size_t* buffer_size,
	ABSTRACT_SYNTAX_TREE_NODE* node,
	STRING_HASH_TABLE* variable_pointer,
	const char* function_name,
	int register_number
)
{
	STRING_HASH_TABLE *variable;
	STRING_HASH_TABLE local_variable;
	TOKEN *token;
	TOKEN *variable_token;
	size_t code_size = 0;
	uint32 *code_pointer;

	if(variable_pointer == NULL && function_name != NULL)
	{
		InitializeStringHashTable(&local_variable, 1024, NULL);
		variable_pointer = &local_variable;
	}
	else
	{
		if(function_name == NULL)
		{
			variable = &compiler->global;
		}
		else
		{
			variable = variable_pointer;
		}
	}

	token = node->token;
	switch(token->token_type)
	{
	case TOKEN_TYPE_ASSIGN:
		code_size += AbstractSyntaxTreeToByteCodeLocal(compiler, code, buffer_size,
			node->child[1], variable_pointer, function_name);
		if(code + sizeof(uint32) + sizeof(uint32) >= buffer_size)
		{
			buffer_size += CODE_BLOCK_SIZE;
			*code = (uint8*)MEM_REALLOC_FUNC(*code, buffer_size);
		}

		variable_token = StringHashTableGet(variable, node->child[0]->token->name);
		*code_pointer = (function_name == NULL) ? SCRIPT_OPERAND_GLOBAL_STORE : SCRIPT_OPERAND_HEAP_STORE;
		code_size += sizeof(*code_pointer);
		code_pointer = (uint32)&code[code_size];
		if(variable_token != NULL)
		{
			*code_pointer = (uint32)((uint8*)variable_token - (uint8*)variable->buffer);
		}
		else
		{
			*code_pointer = (uint32)StringHashTableAppend(variable, variable_token->name);
		}
		code_size += sizeof(*code_pointer);

		break;
	case TOKEN_TYPE_PLUS:
		code_size += AbstractSyntaxTreeToByteCodeLocal(compiler, code, buffer_size,
			node->child[0], variable_pointer, function_name, register_number);
		code_size += AbstractSyntaxTreeToByteCodeLocal(compiler, code, buffer_size,
			node->child[1], variable_pointer, function_name, register_number + 1);
		code_size += sizeof(*code_pointer) + sizeof(*code_pointer);

		break;
	default:
		goto ERROR_END;
	}

	if(variable_pointer == NULL && function_name != NULL)
	{
		ReleaseStringHashTable(&local_variable);
	}

	return code_size;

ERROR_END:
	if(variable_pointer == NULL && function_name != NULL)
	{
		ReleaseStringHashTable(&local_variable);
	}
	MEM_FREE_FUNC(*code);
	*code = NULL;
	return 0;
}

static size_t AbstractSyntaxTreeToByteCode(SCRIPT_COMPILER* compiler, uint8** code)
{
	ABSTRACT_SYNTAX_TREE_NODE *node;
	TOKEN *token;
	size_t buffer_size = CODE_BLOCK_SIZE;
	size_t code_size = 0;
	uint32 operand;
	int num_tree = (int)compiler->abstract_syntax_tree->num_data;
	int i;

	*code = (uint8*)MEM_ALLOC_FUNC(buffer_size);

	for(i=0; i<num_tree; i++)
	{
		node = (ABSTRACT_SYNTAX_TREE_NODE*)compiler->abstract_syntax_tree->buffer[i];
		token = node->token;

		aa
	}

	return code_size;

ERROR_END:
	MEM_FREE_FUNC(*code);
	*code = NULL;
	return 0;
}

uint8* CompileScript(SCRIPT_COMPILER* compiler)
{
	uint8 *code;
	uint8 *result = NULL;
	size_t code_size;

	code_size = AbstractSyntaxTreeToByteCode(compiler, &code);

	if(code_size > 0)
	{
		result = (uint8*)MemoryPoolAllocate(compiler->memory_pool, code_size);
		(void)memcpy(result, code, code_size);
	}

	MEM_FREE_FUNC(code);

	return result;
}

#endif
