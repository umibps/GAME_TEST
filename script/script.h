#ifndef _INCLUDED_SCRIPT_H_
#define _INCLUDED_SCRIPT_H_

#include "rule.h"
#include "parser.h"
#include "lexical_analyser.h"

/*
 SCRIPT_BASIC_EXECUTOR構造体
 デフォルトのスクリプト実行用データ
*/
typedef struct _SCRIPT_BASIC_EXECUTOR
{
	LEXICAL_ANALYSER analyser;
	SCRIPT_BASIC_RULE rule;
	SCRIPT_BASIC_PARSER parser;
	const char **user_function_names;
	int num_user_functions;
	void *user_data;
	MEMORY_POOL memory_pool;
} SCRIPT_BASIC_EXECUTOR;

#ifdef __cplusplus
extern "C" {
#endif

/*
 DefaultScriptErrorMessage関数
 デフォルトのスクリプト処理中のエラー表示関数
 引数
 dummy
 file_name	: エラーのあったファイル名
 line		: エラーの発生した行数 (0の場合はスクリプトの内容以外のエラー)
 message	: エラーメッセージ
 ... : printfと同じ
*/
EXTERN void DefaultScriptErrorMessage(
	void* dummy,
	const char* file_name,
	int line,
	const char* message,
	...
);

/*
 InitializreScriptBasicExecutor関数
 デフォルトのスクリプト実行用データを初期化
 引数
 executor				: デフォルトのスクリプト実行用データ
 file_path				: 実行するスクリプトのファイルパス
 user_function_names	: ユーザー定義関数の名前配列
 num_user_functions		: ユーザー定義関数の数
 user_data				: ユーザー定義関数で使うユーザーデータ
 error_message			: エラーメッセージ表示用関数
 error_message_data		: エラーメッセージ表示用関数で使うユーザーデータ
*/
EXTERN void InitializeScriptBasicExecutor(
	SCRIPT_BASIC_EXECUTOR* executor,
	const char* file_path,
	const char** user_function_names,
	int num_user_functions,
	void* user_data,
	void (*error_message)(void*, const char*, ...),
	void* error_message_data
);

/*
 ReleaseScriptBasicExecutor関数
 デフォルトのスクリプト実行用データのメモリ開放
 引数
 executor	: デフォルトのスクリプト実行用データ
*/
EXTERN void ReleaseScriptBasicExecutor(SCRIPT_BASIC_EXECUTOR* executor);

/*
 ScriptBasicEcecutorExecute関数
 デフォルトのスクリプト実行用データでスクリプト実行
 引数
 executor	: デフォルトのスクリプト実行用データ
 open_func	: ソースコードデータを開くための関数ポインタ
 read_func	: ソースコードデータを読み込むための関数ポインタ
 seek_func	: ソースコードデータをシークするための関数ポインタ
 tell_func	: ソースコードデータのシーク位置を取得するための関数ポインタ
 close_func	: ソースコードデータを閉じるための関数ポインタ
 open_data	: ソースコードを開く際に使う外部データ
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
EXTERN int ScriptBasicExecutorExecute(
	SCRIPT_BASIC_EXECUTOR* executor,
	void* (*open_func)(const char*, const char*, void*),
	size_t (*read_func)(void*, size_t, size_t, void*),
	void* (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	int (*close_func)(void*),
	void* open_data
);

/*
 ReleaseScriptBasicExecutor関数
 デフォルトのスクリプト実行用データのメモリ開放
 引数
 executor	: デフォルトのスクリプト実行用データ
*/
EXTERN void ReleaseScriptBasicExecutor(SCRIPT_BASIC_EXECUTOR* executor);

/*
 ScriptBasicGetDefaultReservedStrings関数
 デフォルトの予約語の文字列配列を取得する
 引数
 num_strings	: 予約語の数
 返り値
	デフォルトの予約語の文字列配列
*/
EXTERN const char** ScriptBasicGetDefaultReservedStrings(int* num_strings);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SCRIPT_H_
