#include <stdio.h>
#include <ctype.h>

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
int StringCompareIgnoreCase(const char* str1, const char* str2)
{
	int ret;

	while((ret = toupper(*str1) - toupper(*str2)) == 0)
	{
		str1++, str2++;
		if(*str1 == '\0')
		{
			return 0;
		}
	}

	return ret;
}

/*
 GetNextUtf8Character関数
 次のUTF8の文字を取得する
 引数
 str	: 次の文字を取得したい文字列
 返り値
	次の文字のアドレス
*/
const char* GetNextUtf8Character(const char* str)
{
	static const unsigned char skips[256] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
	};

	return str + skips[((unsigned char)*str)];
}

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
FILE* FileOpen(const char* path, const char* mode, void* user_data)
{
	return fopen(path, mode);
}

/*
 FileClose関数
 ファイルを閉じる
 引数
 fp			: ファイルポインタ
 user_data	: プログラマーが自由に使えるデータ
 返り値
	常に0
*/
int FileClose(FILE* fp, void* user_data)
{
	(void)fclose(fp);

	return 0;
}

#ifdef __cplusplus
}
#endif
