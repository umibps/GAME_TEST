#ifndef _INCLUDED_LEXICAL_ANALYSER_
#define _INCLUDED_LEXICAL_ANALYSER_

#include <stdarg.h>
#include "token.h"
#include "../memory_pool.h"
#include "../utils.h"

/*
 LEXICAL_ANALYSER構造体
 ソースコードをトークンに分解する
*/
typedef struct _LEXICAL_ANALYSER
{
	char *file_path;				// ソースコードのファイルパス
	MEMORY_POOL memory_pool;		// トークン保持用のメモリプール
	POINTER_ARRAY file_names;		// ソースコードファイル
	POINTER_ARRAY tokens;			// トークン
	// エラーメッセージ表示用関数
	void (*error_message_func)(void* data, const char* message, ... );
	void *error_message_func_data;	// エラー表示用の関数で使うデータ
} LEXICAL_ANALYSER;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeLexicalAnalyzer関数
 ソースコードをトークンに分解するためのデータを初期化
 引数
 analyser			: ソースコードをトークンに分解するためのデータ
 file_path			: ソースコードのパス
 error_message_func	: エラーメッセージ表示用関数ポインタ
 message_func_data	: エラーメッセージ表示用関数で使うデータ
*/
EXTERN void InitializeLexicalAnalyzer(
	LEXICAL_ANALYSER* analyser,
	const char* file_path,
	void (*error_message_func)(void*, const char*, ... ),
	void* message_func_data
);

/*
 ReleaseLexicalAnalyzer関数
 ソースコードをトークンに分解するためのデータを開放する
 引数
 analyser	: ソースコードをトークンに分解するためのデータ
*/
EXTERN void ReleaseLexicalAnalyzer(LEXICAL_ANALYSER* analyser);

/*
 LexicalAnayze関数
 ソースコードをトークンに分解する
 引数
 analyser	: ソースコードをトークンに分解するためのデータ
 open_func	: ソースコードデータを開くための関数ポインタ
 read_func	: ソースコードデータを読み込むための関数ポインタ
 seek_func	: ソースコードデータをシークするための関数ポインタ
 tell_func	: ソースコードデータのシーク位置を取得するための関数ポインタ
 close_func	: ソースコードデータを閉じるための関数ポインタ
 open_data	: ソースコードを開く際に使う外部データ
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
EXTERN int LexicalAnalyze(
	LEXICAL_ANALYSER* analyser,
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

#endif	// #ifndef _INCLUDED_LEXICAL_ANALYSER_
