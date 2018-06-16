#include <string.h>
#include "compile.h"
#include "token.h"
#include "parser.h"
#include "operand.h"
#include "script.h"

#define CODE_BLOCK_SIZE 4096

#ifdef __cplusplus
extern "C" {
#endif

/*
 ScriptBasicCompilerByteCodeFromAbstractSyntaxTree�֐�
 �\����͂ɂ���ē���ꂽ���ۍ\���،Q���o�C�g�R�[�h�ɕϊ�����
 ����
 compiler	: �X�N���v�g�̃R���p�C��
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicCompilerByteCodeFromAbstractSyntaxTree(SCRIPT_BASIC_COMPILER* compiler);

/*
 InitializeScriptBasicCompiler�֐�
 �f�t�H���g�̃X�N���v�g�̃R���p�C��������������
 ����
 compiler				: �R���p�C���̃f�[�^
 file_path				: ���s����X�N���v�g�̃t�@�C���p�X
 user_function_names	: ���[�U�[��`�֐��̖��O�z��
 num_user_functions		: ���[�U�[��`�֐��̐�
 user_data				: ���[�U�[��`�֐��Ŏg�����[�U�[�f�[�^
 error_message			: �G���[���b�Z�[�W�\���p�֐�
 error_message_data		: �G���[���b�Z�[�W�\���p�֐��Ŏg�����[�U�[�f�[�^
*/
void InitializeScriptBasicCompiler(
	SCRIPT_BASIC_COMPILER* compiler,
	const char* file_path,
	const char** user_function_names,
	int num_user_functions,
	void* user_data,
	void (*error_message)(void*, const char*, ...),
	void* error_message_data
)
{
#define MEMORY_BLOCK_SIZE 4096
#define USER_FUNCTION_BLOCK_SIZE 128
	int i;
	(void)memset(compiler, 0, sizeof(*compiler));
	InitializeStringHashTable(&compiler->user_function_names, USER_FUNCTION_BLOCK_SIZE, NULL);
	for(i=0; i<num_user_functions; i++)
	{
		(void)StringHashTableAppend(&compiler->user_function_names, user_function_names[i], (void*)(i+1));
	}
	compiler->user_data = user_data;

	InitializeLexicalAnalyser(&compiler->analyser, file_path, error_message, error_message_data);
	InitializeMemoryPool(&compiler->memory_pool, MEMORY_BLOCK_SIZE);
	InitializeByteArray(&compiler->byte_code, MEMORY_BLOCK_SIZE);
	InitializePointerArray(&compiler->jump_buffer, MEMORY_BLOCK_SIZE / sizeof(void*), NULL);
#undef MEMORY_BLOCK_SIZE
}

/*
 ReleaseScriptBasiccompiler�֐�
 �f�t�H���g�̃X�N���v�g�̃R���p�C���f�[�^�̃������J��
 ����
 compiler	: �f�t�H���g�̃X�N���v�g�̃R���p�C���f�[�^
*/
void ReleaseScriptBasicCompiler(SCRIPT_BASIC_COMPILER* compiler)
{
	ReleaseLexicalAnalyser(&compiler->analyser);
	compiler->parser.element.release(&compiler->parser.element);
	ReleaseStringHashTable(&compiler->user_function_names);
	ReleaseMemoryPool(&compiler->memory_pool);
	ReleaseByteArray(&compiler->byte_code);
	ReleasePointerArray(&compiler->jump_buffer);
}

/*
 ScriptBasicCompilerCompile�֐�
 �f�t�H���g�̃X�N���v�g�R���p�C���ŃR���p�C�����s
 ����
 compiler	: �f�t�H���g�̃X�N���v�g�̃R���p�C���f�[�^
 open_func	: �\�[�X�R�[�h�f�[�^���J�����߂̊֐��|�C���^
 read_func	: �\�[�X�R�[�h�f�[�^��ǂݍ��ނ��߂̊֐��|�C���^
 seek_func	: �\�[�X�R�[�h�f�[�^���V�[�N���邽�߂̊֐��|�C���^
 tell_func	: �\�[�X�R�[�h�f�[�^�̃V�[�N�ʒu���擾���邽�߂̊֐��|�C���^
 close_func	: �\�[�X�R�[�h�f�[�^����邽�߂̊֐��|�C���^
 open_data	: �\�[�X�R�[�h���J���ۂɎg���O���f�[�^
 �Ԃ�l
	�R���p�C���̌��ʁA�������ꂽ�o�C�g�R�[�h	�ُ�I������NULL
	�s�v�ɂȂ�����MEM_FREE_FUNC����
*/
uint8* ScriptBasicCompilerCompile(
	SCRIPT_BASIC_COMPILER* compiler,
	void* (*open_func)(const char*, const char*, void*),
	size_t (*read_func)(void*, size_t, size_t, void*),
	void* (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	int (*close_func)(void*),
	void* open_data
)
{
	uint8 *byte_code = NULL;
	const char **reserved_string;
	uint16 reserved_ids[] = {SCRIPT_BASIC_RESERVED_IF, SCRIPT_BASIC_RESERVED_ELSE};
	int (*rules[])(struct _SCRIPT_RULE_ELEMENT* rule, struct _LEXICAL_ANALYSER* analyser, void* function_data)
		= {ScriptBasicIfRule, ScriptBasicElseRule, ScriptBasicWhileRule, ScriptBasicBreakRule};
	int (*parse_functions[])(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
		= {ScriptBasicParserParseIf, ScriptBasicParserParseElse, ScriptBasicParserParseWhile, ScriptBasicParserParseBreak};
	int num_strings;

	reserved_string = ScriptBasicGetDefaultReservedStrings(&num_strings);
	LexicalAnalyserSetReserved(&compiler->analyser, reserved_string,
		reserved_ids, sizeof(rules) / sizeof(*rules));

	if(LexicalAnalyse(&compiler->analyser, open_func, read_func,
		seek_func, tell_func, close_func, open_data) == FALSE)
	{
		return NULL;
	}

	InitializeScriptBasicRule(&compiler->rule, compiler->analyser.error_message_func,
		compiler->analyser.error_message_func_data, (const char**)compiler->analyser.file_names.buffer, &compiler->memory_pool);
	InitializeScriptBasicParser(&compiler->parser, &compiler->rule.element,
		(TOKEN**)compiler->analyser.tokens.buffer, (int)compiler->analyser.tokens.num_data,
			&compiler->user_function_names
	);
	ScriptRuleElementSetReservedRule(&compiler->rule.element,
		rules, sizeof(rules) / sizeof(*rules), NULL);
	ScriptParserElementSetReservedParseRule(&compiler->parser.element,
		parse_functions);

	if(compiler->parser.element.parse(&compiler->parser.element) == FALSE)
	{
		return NULL;
	}

	if(ScriptBasicCompilerByteCodeFromAbstractSyntaxTree(compiler) == FALSE)
	{
		return NULL;
	}

	byte_code = (uint8*)MEM_ALLOC_FUNC(compiler->byte_code.num_data);
	(void)memcpy(byte_code, compiler->byte_code.buffer, compiler->byte_code.num_data);

	return byte_code;
}

typedef struct _SCRIPT_BASIC_COMPILER_BLOCK
{
#define MAX_VARIABLE 128
	struct _SCRIPT_BASIC_COMPILER_BLOCK *parent;
	char *variable_names[MAX_VARIABLE];
	int num_variable;
	int num_child_variable;
} SCRIPT_BASIC_COMPILER_BLOCK;

typedef struct _FIND_VARIABLE
{
	SCRIPT_BASIC_COMPILER_BLOCK *block;
	script_index index;
} FIND_VARIABLE;

/*
 FindVariable�֐�
 �w�肳�ꂽ���O�̃O���[�o���ϐ��܂��̓��[�J���ϐ���T��
 ����
 block_variable	: �u���b�N���ێ�����ϐ����
 target			: �ϐ��̖��O
 �Ԃ�l
	�w�肳�ꂽ���O�̕ϐ������u���b�N�ƕϐ���ID
*/
static FIND_VARIABLE FindVariable(
	SCRIPT_BASIC_COMPILER_BLOCK* block_variable,
	const char* target
)
{
	FIND_VARIABLE find = {0};
	int i;

	do
	{
		for(i=0; i<block_variable->num_variable; i++)
		{
			if(strcmp(block_variable->variable_names[i],target) == 0)
			{
				find.block = block_variable;
				find.index = i;
				return find;
			}
		}
		block_variable = block_variable->parent;
	} while(block_variable != NULL);

	return find;
}

/*
 GetVariableIndexOffset�֐�
 �ϐ���ID�v�Z�p�̃I�t�Z�b�g���擾����
 ����
 block_variable	: ���݂̃u���b�N
 �Ԃ�l
	�I�t�Z�b�g�l
*/
static int GetVariableIndexOffset(SCRIPT_BASIC_COMPILER_BLOCK* block_variable)
{
	int index = 0;

	while(block_variable->parent != NULL)
	{
		index += block_variable->num_variable;
		block_variable = block_variable->parent;
	}

	return index;
}

/*
 AddBinaryOperation�֐�
 compiler			: �X�N���v�g�̃R���p�C��
 operand			: ���߂̎��ʎq
 register_index1	: ���W�X�^�[1�̃C���f�b�N�X
 register_index2	: ���W�X�^�[2�̃C���f�b�N�X
*/
static void AddBinaryOperation(
	SCRIPT_BASIC_COMPILER* compiler,
	script_operand operand,
	script_index register_index1,
	script_index register_index2
)
{
	ByteArrayAppend(&compiler->byte_code, (uint8*)&operand, sizeof(operand));
	ByteArrayAppend(&compiler->byte_code, (uint8*)&register_index1, sizeof(register_index1));
	ByteArrayAppend(&compiler->byte_code, (uint8*)&register_index2, sizeof(register_index2));
}

/*
 ScriptBasicCompilerTraverseAbstractSyntaxTree�֐�
 ���ۍ\���؂��o�C�g�R�[�h�ɕϊ�����
 ����
 compiler		: �X�N���v�g�̃R���p�C��
 node			: ���ۍ\���؂̗t
 block_variable	: �u���b�N���ێ�����ϐ��̏��
 register_index	: ���W�X�^�[�̃C���f�b�N�X
 id				: ���ۍ\���؂̃C���f�b�N�X
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicCompilerTraverseAbstractSyntaxTree(
	SCRIPT_BASIC_COMPILER* compiler,
	ABSTRACT_SYNTAX_TREE* node,
	SCRIPT_BASIC_COMPILER_BLOCK* block_variable,
	int register_index,
	int* id
)
{
	if(node->token->token_type != SCRIPT_BASIC_RESERVED_USER_FUNCTION
		&& node->token->token_type != SCRIPT_BASIC_RESERVED_IF
		&& node->token->token_type != SCRIPT_BASIC_RESERVED_WHILE
	)
	{
		if(node->token->token_type != TOKEN_TYPE_ASSIGN && node->left != NULL)
		{
			if(ScriptBasicCompilerTraverseAbstractSyntaxTree(
				compiler, node->left, block_variable, register_index, id) == FALSE)
			{
				return FALSE;
			}
		}
		if(node->center != NULL)
		{
			if(ScriptBasicCompilerTraverseAbstractSyntaxTree(
				compiler, node->center, block_variable, register_index + 1, id) == FALSE)
			{
				return FALSE;
			}
		}
		if(node->right != NULL)
		{
			if(ScriptBasicCompilerTraverseAbstractSyntaxTree(
				compiler, node->right, block_variable, node->center == NULL ? register_index + 1 : register_index + 2, id) == FALSE)
			{
				return FALSE;
			}
		}
	}

	switch(node->token->token_type)
	{
	case TOKEN_TYPE_ASSIGN:
		{
			FIND_VARIABLE find;
			script_operand operand;
			find = FindVariable(block_variable, node->left->token->name);
			if(find.block == NULL)
			{
				block_variable->variable_names[block_variable->num_variable] =
					node->left->token->name;
				find.block = block_variable;
				find.index = block_variable->num_variable +
					GetVariableIndexOffset(block_variable);
				block_variable->num_variable++;
			}
			operand = (find.block->parent == NULL) ?
				SCRIPT_OPERAND_GLOBAL_STORE : SCRIPT_OPERAND_STORE;
			AddBinaryOperation(compiler, operand, register_index, find.index);
		}
		break;
	case TOKEN_TYPE_PLUS:
		AddBinaryOperation(compiler, SCRIPT_OPERAND_PLUS, register_index, register_index + 1);
		break;
	case TOKEN_TYPE_MINUS:
		AddBinaryOperation(compiler, SCRIPT_OPERAND_MINUS, register_index, register_index + 1);
		break;
	case TOKEN_TYPE_MULTI:
		AddBinaryOperation(compiler, SCRIPT_OPERAND_MULTI, register_index, register_index + 1);
		break;
	case TOKEN_TYPE_DIVIDE:
		AddBinaryOperation(compiler, SCRIPT_OPERAND_DIVIDE, register_index, register_index + 1);
		break;
	case TOKEN_TYPE_MOD:
		AddBinaryOperation(compiler, SCRIPT_OPERAND_MOD, register_index, register_index + 1);
		break;
	case TOKEN_TYPE_EQUAL:
		AddBinaryOperation(compiler, SCRIPT_OPERAND_EQUAL, register_index, register_index + 1);
		break;
	case TOKEN_TYPE_NOT_EQUAL:
		AddBinaryOperation(compiler, SCRIPT_OPERAND_NOT_EQUAL, register_index, register_index + 1);
		break;
	case TOKEN_TYPE_LESS:
		AddBinaryOperation(compiler, SCRIPT_OPERAND_LESS, register_index, register_index + 1);
		break;
	case TOKEN_TYPE_LESS_EQUAL:
		AddBinaryOperation(compiler, SCRIPT_OPERAND_LESS_EQUAL, register_index, register_index + 1);
		break;
	case TOKEN_TYPE_GREATER:
		AddBinaryOperation(compiler, SCRIPT_OPERAND_GREATER, register_index, register_index + 1);
		break;
	case TOKEN_TYPE_GREATER_EQUAL:
		AddBinaryOperation(compiler, SCRIPT_OPERAND_GREATER_EQUAL, register_index, register_index + 1);
		break;
	case TOKEN_TYPE_CONST_DIGIT:
		{
			script_operand operand = SCRIPT_OPERAND_LOAD_CONST_INTEGER;
			script_int value = atoi(node->token->name);
			ByteArrayAppend(&compiler->byte_code, (uint8*)&operand, sizeof(operand));
			ByteArrayAppend(&compiler->byte_code, (uint8*)&register_index, sizeof(register_index));
			ByteArrayAppend(&compiler->byte_code, (uint8*)&value, sizeof(value));
		}
		break;
	case TOKEN_TYPE_CONST_FLOAT:
		{
			script_operand operand = SCRIPT_OPERAND_LOAD_CONST_FLOAT;
			script_float value = atof(node->token->name);
			ByteArrayAppend(&compiler->byte_code, (uint8*)&operand, sizeof(operand));
			ByteArrayAppend(&compiler->byte_code, (uint8*)&register_index, sizeof(register_index));
			ByteArrayAppend(&compiler->byte_code, (uint8*)&value, sizeof(value));
		}
		break;
	case TOKEN_TYPE_CONST_STRING:
		{
			script_operand operand = SCRIPT_OPERAND_LOAD_CONST_STRING;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&operand, sizeof(operand));
			ByteArrayAppend(&compiler->byte_code, (uint8*)&register_index, sizeof(register_index));
			ByteArrayAppend(&compiler->byte_code, (uint8*)node->token->name, strlen(node->token->name)+1);
		}
		break;
	case TOKEN_TYPE_IDENT:
		{
			FIND_VARIABLE find;
			script_operand operand;
			find = FindVariable(block_variable, node->token->name);
			if(find.block != NULL)
			{
				operand = (find.block->parent == NULL) ? SCRIPT_OPERAND_GLOBAL_RESTORE : SCRIPT_OPERAND_RESTORE;
				ByteArrayAppend(&compiler->byte_code, (uint8*)&operand, sizeof(operand));
				ByteArrayAppend(&compiler->byte_code, (uint8*)&register_index, sizeof(register_index));
				ByteArrayAppend(&compiler->byte_code, (uint8*)&find.index, sizeof(find.index));
			}
		}
		break;
	case TOKEN_TYPE_LEFT_BRACE:
		{
			ABSTRACT_SYNTAX_TREE **trees = (ABSTRACT_SYNTAX_TREE**)compiler->parser.element.abstract_syntax_tree.buffer;
			SCRIPT_BASIC_COMPILER_BLOCK child = {0};
			child.parent = block_variable;
			(*id)++;
			while(trees[*id]->token->token_type != TOKEN_TYPE_RIGHT_BRACE)
			{
				if(ScriptBasicCompilerTraverseAbstractSyntaxTree(compiler,trees[*id],&child,register_index,id) == FALSE)
				{
					return FALSE;
				}
				(*id)++;
			}
			if((trees[*id]->token->token_type != TOKEN_TYPE_RIGHT_BRACE))
			{
				(*id)++;
			}
		}
		break;
	case SCRIPT_BASIC_RESERVED_IF:
		{
			ABSTRACT_SYNTAX_TREE *next_node;
			script_operand operand;
			size_t jump_write;
			script_index jump_to;
			if(ScriptBasicCompilerTraverseAbstractSyntaxTree(compiler,
				node->left, block_variable, register_index, id) == FALSE)
			{
				return FALSE;
			}
			operand = SCRIPT_OPERAND_JUMP_IF_ZERO;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&operand, sizeof(operand));
			jump_to = (script_index)register_index;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&jump_to, sizeof(jump_to));
			// �_�~�[�f�[�^�������o���Ă���
			jump_write = compiler->byte_code.num_data;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&jump_to, sizeof(jump_to));

			// if�̏����������̏����������o��
			(*id)++;
			next_node = (ABSTRACT_SYNTAX_TREE*)compiler->parser.element.abstract_syntax_tree.buffer[*id];
			if(ScriptBasicCompilerTraverseAbstractSyntaxTree(compiler,
				next_node, block_variable, register_index, id) == FALSE)
			{
				return FALSE;
			}
			// if�̏����s�������̃W�����v��������o��
			next_node = (ABSTRACT_SYNTAX_TREE*)compiler->parser.element.abstract_syntax_tree.buffer[*id];
			if((*id) < (int)compiler->parser.element.abstract_syntax_tree.num_data - 1 && next_node->token->token_type == TOKEN_TYPE_RIGHT_BRACE
				&& ((ABSTRACT_SYNTAX_TREE*)compiler->parser.element.abstract_syntax_tree.buffer[*id+1])->token->token_type == SCRIPT_BASIC_RESERVED_ELSE)
			{
				(*id)++;
				next_node = (ABSTRACT_SYNTAX_TREE*)compiler->parser.element.abstract_syntax_tree.buffer[*id];
			}
			if(next_node->token->token_type != SCRIPT_BASIC_RESERVED_ELSE)
			{
				jump_to = (script_index)compiler->byte_code.num_data;
				compiler->byte_code.num_data = jump_write;
				ByteArrayAppend(&compiler->byte_code, (uint8*)&jump_to, sizeof(jump_to));
				compiler->byte_code.num_data = (size_t)jump_to;
			}
			else
			{
				operand = SCRIPT_OPERAND_JUMP;
				ByteArrayAppend(&compiler->byte_code, (uint8*)&operand, sizeof(operand));
				// �_�~�[�f�[�^�������o���Ă���
				ByteArrayAppend(&compiler->byte_code, (uint8*)&jump_to, sizeof(jump_to));
				jump_to = (script_index)compiler->byte_code.num_data;
				compiler->byte_code.num_data = jump_write;
				ByteArrayAppend(&compiler->byte_code, (uint8*)&jump_to, sizeof(jump_to));
				compiler->byte_code.num_data = (size_t)jump_to;
				jump_write = jump_to - sizeof(jump_to);
				(*id)++;
				next_node = (ABSTRACT_SYNTAX_TREE*)compiler->parser.element.abstract_syntax_tree.buffer[*id];
				if(ScriptBasicCompilerTraverseAbstractSyntaxTree(compiler,
					next_node, block_variable, register_index, id) == FALSE)
				{
					return FALSE;
				}
				next_node = (ABSTRACT_SYNTAX_TREE*)compiler->parser.element.abstract_syntax_tree.buffer[*id];
				if(next_node->token->token_type != TOKEN_TYPE_RIGHT_BRACE)
				{
					(*id)--;
				}
				jump_to = (script_index)compiler->byte_code.num_data;
				compiler->byte_code.num_data = (size_t)jump_write;
				ByteArrayAppend(&compiler->byte_code, (uint8*)&jump_to, sizeof(jump_to));
				compiler->byte_code.num_data = (size_t)jump_to;
			}
		}
		break;
	case SCRIPT_BASIC_RESERVED_WHILE:
		{
			ABSTRACT_SYNTAX_TREE *next_node;
			script_operand operand;
			size_t jump_write;
			script_index loop_start;
			script_index jump_to;
			size_t break_start = compiler->jump_buffer.num_data;
			int i;

			loop_start = (script_index)compiler->byte_code.num_data;
			if(ScriptBasicCompilerTraverseAbstractSyntaxTree(compiler,
				node->left, block_variable, register_index, id) == FALSE)
			{
				return FALSE;
			}
			operand = SCRIPT_OPERAND_JUMP_IF_ZERO;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&operand, sizeof(operand));
			jump_to = (script_index)register_index;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&jump_to, sizeof(jump_to));
			// �_�~�[�f�[�^�������o���Ă���
			jump_write = compiler->byte_code.num_data;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&jump_to, sizeof(jump_to));

			// while�̃��[�v�������������o��
			(*id)++;
			next_node = (ABSTRACT_SYNTAX_TREE*)compiler->parser.element.abstract_syntax_tree.buffer[*id];
			if(ScriptBasicCompilerTraverseAbstractSyntaxTree(compiler,
				next_node, block_variable, register_index, id) == FALSE)
			{
				return FALSE;
			}
			// ���[�v�̊J�n�ʒu�ɖ߂鏈��
			operand = SCRIPT_OPERAND_JUMP;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&operand, sizeof(operand));
			ByteArrayAppend(&compiler->byte_code, (uint8*)&loop_start, sizeof(loop_start));
			// ���[�v�𔲂��o������
			jump_to = (script_index)compiler->byte_code.num_data;
			compiler->byte_code.num_data = (size_t)jump_write;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&jump_to, sizeof(jump_to));

			for(i=(int)break_start; i<(int)compiler->jump_buffer.num_data; i++)
			{
				uintptr_t write_position = (uintptr_t)compiler->jump_buffer.buffer[i];
				compiler->byte_code.num_data = (size_t)write_position;
				ByteArrayAppend(&compiler->byte_code, (uint8*)&jump_to, sizeof(jump_to));
			}
			compiler->byte_code.num_data = (size_t)jump_to;
			compiler->jump_buffer.num_data = break_start;
		}
		break;
	case SCRIPT_BASIC_RESERVED_BREAK:
		{
			script_operand operand = SCRIPT_OPERAND_JUMP;
			script_index dummy = 0;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&operand, sizeof(operand));
			PointerArrayAppend(&compiler->jump_buffer, (void*)compiler->byte_code.num_data);
			ByteArrayAppend(&compiler->byte_code, (uint8*)&dummy, sizeof(dummy));
		}
		break;
	case SCRIPT_BASIC_RESERVED_USER_FUNCTION:
		{
			ABSTRACT_SYNTAX_TREE **trees = (ABSTRACT_SYNTAX_TREE**)compiler->parser.element.abstract_syntax_tree.buffer;
			intptr_t index = (intptr_t)node->left - 1;
			intptr_t loop = (intptr_t)node->right;
			intptr_t waste_return = (intptr_t)node->center;
			script_index data = (script_index)waste_return;
			int i;

			for(i=1; i<=loop; i++)
			{
				if(ScriptBasicCompilerTraverseAbstractSyntaxTree(compiler,trees[(*id)+i],block_variable,register_index + i,id) == FALSE)
				{
					return FALSE;
				}
			}
			AddBinaryOperation(compiler, SCRIPT_OPERAND_CALL_USER_FUNCTION,
				(script_index)index, (script_index)loop);
			ByteArrayAppend(&compiler->byte_code, (uint8*)&data, sizeof(data));
			(*id) += (int)loop;
		}
		break;
	}
	return TRUE;
}

/*
 ScriptBasicCompilerByteCodeFromAbstractSyntaxTree�֐�
 �\����͂ɂ���ē���ꂽ���ۍ\���،Q���o�C�g�R�[�h�ɕϊ�����
 ����
 compiler	: �X�N���v�g�̃R���p�C��
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicCompilerByteCodeFromAbstractSyntaxTree(SCRIPT_BASIC_COMPILER* compiler)
{
	ABSTRACT_SYNTAX_TREE **trees = (ABSTRACT_SYNTAX_TREE**)compiler->parser.element.abstract_syntax_tree.buffer;
	SCRIPT_BASIC_COMPILER_BLOCK block_variable = {0};
	const int loop = (int)compiler->parser.element.abstract_syntax_tree.num_data;
	int id;

	for(id=0; id < loop; id++)
	{
		ABSTRACT_SYNTAX_TREE *node = (ABSTRACT_SYNTAX_TREE*)trees[id];
		if(ScriptBasicCompilerTraverseAbstractSyntaxTree(compiler, trees[id], &block_variable, 0, &id) == FALSE)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/*
 ScriptBasicCompilerGetByteCodeSize�֐�
 �R���p�C���̌��ʓ���ꂽ�o�C�g�R�[�h�̃T�C�Y���擾����
 ����
 compiler	: �X�N���v�g�̃R���p�C��
 �Ԃ�l
	�o�C�g�R�[�h�̃T�C�Y
*/
size_t ScriptBasicCompilerGetByteCodeSize(SCRIPT_BASIC_COMPILER* compiler)
{
	return compiler->byte_code.num_data;
}

#ifdef __cplusplus
}
#endif
