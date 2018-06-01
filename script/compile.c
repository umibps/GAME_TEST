#include <string.h>
#include "compile.h"
#include "token.h"
#include "parser.h"
#include "operand.h"
#include "script.h"

#define CODE_BLOCK_SIZE 4096

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
	正常終了:TRUE	異常終了:FALSE
*/
int ScriptBasicCompilerCompile(
	SCRIPT_BASIC_COMPILER* compiler,
	void* (*open_func)(const char*, const char*, void*),
	size_t (*read_func)(void*, size_t, size_t, void*),
	void* (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	int (*close_func)(void*),
	void* open_data
)
{
	const char **reserved_string;
	uint16 reserved_ids[] = {SCRIPT_BASIC_RESERVED_IF, SCRIPT_BASIC_RESERVED_ELSE};
	int (*rules[])(struct _SCRIPT_RULE_ELEMENT* rule, struct _LEXICAL_ANALYSER* analyser, void* function_data)
		= {ScriptBasicIfRule, ScriptBasicElseRule};
	int (*parse_functions[])(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
		= {ScriptBasicParserParseIf, ScriptBasicParserParseElse};
	int num_strings;

	reserved_string = ScriptBasicGetDefaultReservedStrings(&num_strings);
	LexicalAnalyserSetReserved(&compiler->analyser, reserved_string,
		reserved_ids, sizeof(rules) / sizeof(*rules));

	if(LexicalAnalyse(&compiler->analyser, open_func, read_func,
		seek_func, tell_func, close_func, open_data) == FALSE)
	{
		return FALSE;
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
		return FALSE;
	}

	return TRUE;
}
