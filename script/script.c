#include <stdarg.h>
#include "script.h"

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
 ...		: printfと同じ
*/
void DefaultScriptErrorMessage(
	void* dummy,
	const char* file_name,
	int line,
	const char* message,
	...
)
{
	va_list list;

	if(line > 0)
	{
		(void)fprintf(stderr, "At line %d, in %s\n", line, file_name);
	}

	va_start(list, message);

	(void)vfprintf(stderr, message, list);

	va_end(list);
}

/*
 ScriptBasicGetDefaultReservedStrings関数
 デフォルトの予約語の文字列配列を取得する
 引数
 num_strings	: 予約語の数
 返り値
	デフォルトの予約語の文字列配列
*/
const char** ScriptBasicGetDefaultReservedStrings(int* num_strings)
{
	static const char *reserved[] ={"if", "else", "while", "break"};
	if(num_strings != NULL)
	{
		*num_strings = sizeof(reserved) / sizeof(char*);
	}

	return reserved;
}

#ifdef __cplusplus
}
#endif
