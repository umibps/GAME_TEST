#ifndef _INCLUCED_COMPILE_H_
#define _INCLUCED_COMPILE_H_

#include <stdarg.h>
#include "lexical_analyser.h"
#include "rule.h"
#include "parser.h"
#include "../utils.h"
#include "../memory_pool.h"

/*
 SCRIPT_BASIC_COMPILER構造体
 デフォルトのスクリプトコンパイル用データ
*/
typedef struct _SCRIPT_BASIC_COMPILER
{
	LEXICAL_ANALYSER analyser;
	SCRIPT_BASIC_RULE rule;
	SCRIPT_BASIC_PARSER parser;
	STRING_HASH_TABLE user_function_names;
	void *user_data;
	MEMORY_POOL memory_pool;
} SCRIPT_BASIC_COMPILER;

#ifdef __cplusplus
extern "C" {
#endif

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
EXTERN void InitializeScriptBasicCompiler(
	SCRIPT_BASIC_COMPILER* compiler,
	const char* file_path,
	const char** user_function_names,
	int num_user_functions,
	void* user_data,
	void (*error_message)(void*, const char*, ...),
	void* error_message_data
);

/*
 ReleaseScriptBasiccompiler関数
 デフォルトのスクリプトのコンパイラデータのメモリ開放
 引数
 compiler	: デフォルトのスクリプトのコンパイラデータ
*/
EXTERN void ReleaseScriptBasicCompiler(SCRIPT_BASIC_COMPILER* compiler);

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
EXTERN int ScriptBasicCompilerCompile(
	SCRIPT_BASIC_COMPILER* compiler,
	void* (*open_func)(const char*, const char*, void*),
	size_t (*read_func)(void*, size_t, size_t, void*),
	void* (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	int (*close_func)(void*),
	void* open_data
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUCED_COMPILE_H_
