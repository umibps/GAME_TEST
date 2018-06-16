#ifndef _INCLUDED_SCRIPT_H_
#define _INCLUDED_SCRIPT_H_

#include "rule.h"
#include "parser.h"
#include "lexical_analyser.h"

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
