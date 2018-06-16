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
 ScriptBasicCompilerByteCodeFromAbstractSyntaxTree関数
 構文解析によって得られた抽象構文木群をバイトコードに変換する
 引数
 compiler	: スクリプトのコンパイラ
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicCompilerByteCodeFromAbstractSyntaxTree(SCRIPT_BASIC_COMPILER* compiler);

/*
 InitializeScriptBasicCompiler関数
 デフォルトのスクリプトのコンパイラを初期化する
 引数
 compiler				: コンパイラのデータ
 file_path				: 実行するスクリプトのファイルパス
 user_function_names	: ユーザー定義関数の名前配列
 num_user_functions		: ユーザー定義関数の数
 user_data				: ユーザー定義関数で使うユーザーデータ
 error_message			: エラーメッセージ表示用関数
 error_message_data		: エラーメッセージ表示用関数で使うユーザーデータ
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
 ReleaseScriptBasiccompiler関数
 デフォルトのスクリプトのコンパイラデータのメモリ開放
 引数
 compiler	: デフォルトのスクリプトのコンパイラデータ
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
 ScriptBasicCompilerCompile関数
 デフォルトのスクリプトコンパイラでコンパイル実行
 引数
 compiler	: デフォルトのスクリプトのコンパイラデータ
 open_func	: ソースコードデータを開くための関数ポインタ
 read_func	: ソースコードデータを読み込むための関数ポインタ
 seek_func	: ソースコードデータをシークするための関数ポインタ
 tell_func	: ソースコードデータのシーク位置を取得するための関数ポインタ
 close_func	: ソースコードデータを閉じるための関数ポインタ
 open_data	: ソースコードを開く際に使う外部データ
 返り値
	コンパイルの結果、生成されたバイトコード	異常終了時はNULL
	不要になったらMEM_FREE_FUNCする
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
 FindVariable関数
 指定された名前のグローバル変数またはローカル変数を探す
 引数
 block_variable	: ブロックが保持する変数情報
 target			: 変数の名前
 返り値
	指定された名前の変数を持つブロックと変数のID
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
 GetVariableIndexOffset関数
 変数のID計算用のオフセットを取得する
 引数
 block_variable	: 現在のブロック
 返り値
	オフセット値
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
 AddBinaryOperation関数
 compiler			: スクリプトのコンパイラ
 operand			: 命令の識別子
 register_index1	: レジスター1のインデックス
 register_index2	: レジスター2のインデックス
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
 ScriptBasicCompilerTraverseAbstractSyntaxTree関数
 抽象構文木をバイトコードに変換する
 引数
 compiler		: スクリプトのコンパイラ
 node			: 抽象構文木の葉
 block_variable	: ブロックが保持する変数の情報
 register_index	: レジスターのインデックス
 id				: 抽象構文木のインデックス
 返り値
	正常終了:TRUE	異常終了:FALSE
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
			// ダミーデータを書き出しておく
			jump_write = compiler->byte_code.num_data;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&jump_to, sizeof(jump_to));

			// ifの条件成立時の処理を書き出す
			(*id)++;
			next_node = (ABSTRACT_SYNTAX_TREE*)compiler->parser.element.abstract_syntax_tree.buffer[*id];
			if(ScriptBasicCompilerTraverseAbstractSyntaxTree(compiler,
				next_node, block_variable, register_index, id) == FALSE)
			{
				return FALSE;
			}
			// ifの条件不成立時のジャンプ先を書き出す
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
				// ダミーデータを書き出しておく
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
			// ダミーデータを書き出しておく
			jump_write = compiler->byte_code.num_data;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&jump_to, sizeof(jump_to));

			// whileのループ内処理を書き出す
			(*id)++;
			next_node = (ABSTRACT_SYNTAX_TREE*)compiler->parser.element.abstract_syntax_tree.buffer[*id];
			if(ScriptBasicCompilerTraverseAbstractSyntaxTree(compiler,
				next_node, block_variable, register_index, id) == FALSE)
			{
				return FALSE;
			}
			// ループの開始位置に戻る処理
			operand = SCRIPT_OPERAND_JUMP;
			ByteArrayAppend(&compiler->byte_code, (uint8*)&operand, sizeof(operand));
			ByteArrayAppend(&compiler->byte_code, (uint8*)&loop_start, sizeof(loop_start));
			// ループを抜け出す処理
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
 ScriptBasicCompilerByteCodeFromAbstractSyntaxTree関数
 構文解析によって得られた抽象構文木群をバイトコードに変換する
 引数
 compiler	: スクリプトのコンパイラ
 返り値
	正常終了:TRUE	異常終了:FALSE
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
 ScriptBasicCompilerGetByteCodeSize関数
 コンパイルの結果得られたバイトコードのサイズを取得する
 引数
 compiler	: スクリプトのコンパイラ
 返り値
	バイトコードのサイズ
*/
size_t ScriptBasicCompilerGetByteCodeSize(SCRIPT_BASIC_COMPILER* compiler)
{
	return compiler->byte_code.num_data;
}

#ifdef __cplusplus
}
#endif
