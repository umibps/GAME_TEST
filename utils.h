#ifndef _INCLUDED_UTILS_H_
#define _INCLUDED_UTILS_H_

#include <stdio.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 StringCompareIgnoreCase関数
 大文字/小文字を無視して文字列を比較
 引数
 str1	: 比較する文字列その1
 str2	: 比較する文字列その2
 返り値
	str1 - str2
*/
EXTERN int StringCompareIgnoreCase(const char* str1, const char* str2);

/*
 FileOpen関数
 ファイルを開く
 引数
 path		: ファイルのパス
 mode		: 開くモード(fopen関数に準拠)
 user_data	: プログラマーが自由に使えるデータ
 返り値
	成功:ファイルポインタ	失敗:NULL
*/
EXTERN FILE* FileOpen(const char* path, const char* mode, void* user_data);

/*
 FileClose関数
 ファイルを閉じる
 引数
 fp			: ファイルポインタ
 user_data	: プログラマーが自由に使えるデータ
 返り値
	常に0
*/
EXTERN int FileClose(FILE* fp, void* user_data);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_UTILS_H_
