#ifndef _INCLUDED_TEXT_H_
#define _INCLUDED_TEXT_H_

#include <ft2build.h>
#include FT_FREETYPE_H
#include "types.h"

/*
 eTEXT_IMAGE_SOURCE_TYPE列挙体
 フォントデータのソースタイプ
*/
typedef enum _eTEXT_IMAGE_SOURCE_TYPE
{
	TEXT_IMAGE_SOURCE_TYPE_MEMORY,	// メモリ上
	TEXT_IMAGE_SOURCE_TYPE_FILE		// ファイル
} eTEXT_IMAGE_SOURCE_TYPE;
typedef struct _TEXT_IMAGE
{
	eTEXT_IMAGE_SOURCE_TYPE source_type;	// フォントデータのタイプ(メモリ上またはファイル)
	FT_Library library;						// FreeTypeライブラリを管理するデータ
	FT_Face face;							// フォントのイメージ
	union
	{
		uint8 *buffer;		// メモリ上のフォントデータ
		char *file_path;	// フォントファイルのパス
	} font_data;
} TEXT_IMAGE;

/*
 TEXT_DRAW構造体
 テキスト描画用のデータを管理
*/
typedef struct _TEXT_DRAW
{
	TEXT_IMAGE text_image;	// FreeType関連のデータ
	int width;				// 文字サイズの幅
	int height;				// 文字サイズの高さ
} TEXT_DRAW;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeTextDrawFromFile関数
 テキスト描画用のデータをフォントファイルを使って初期化
 引数
 text_draw	: テキスト描画用のデータ
 file_path	: フォントファイルのパス
 返り値
	成功:TRUE	失敗:FALSE
*/
EXTERN int InitializeTextDrawFromFile(
	TEXT_DRAW* text_draw,
	const char* file_path
);

/*
 InitializeTextDrawFromMemory関数
 テキスト描画用のデータをメモリ上のフォントデータを使って初期化
 引数
 text_draw		: テキスト描画用のデータ
 font_data		: メモリ上のフォントデータ
 font_data_size	: フォントデータのバイト数
 返り値
	成功:TRUE	失敗:FALSE
*/
EXTERN int InitializeTextDrawFromMemory(
	TEXT_DRAW* text_draw,
	uint8* font_data,
	size_t font_data_size
);

/*
 ReleaseTextDraw関数
 テキスト描画用のデータを開放する
 引数
 text_draw	: テキスト描画用のデータ
*/
EXTERN void ReleaseTextDraw(TEXT_DRAW* text_draw);

/*
 TextDrawSetCharacterSize関数
 文字サイズを設定する
 引数
 text_draw	: テキスト描画用のデータ
 width		: 文字サイズ(幅)
 height		: 文字サイズ(高さ)
 返り値
	成功:TRUE	失敗:FALSE
*/
EXTERN int TextDrawSetCharacterSize(
	TEXT_DRAW* text_draw,
	int width,
	int height
);

/*
 TextDrawRender関数
 文字列の描画を実行する
 引数
 text_draw		: テキストの描画を管理するデータ
 utf8_text		: 描画する文字列(UTF-8)
 num_character	: 描画する文字数
 width			: 描画結果の幅を受ける変数のアドレス
 height			: 描画結果の高さを受ける変数のアドレス
 返り値
	成功:ピクセルデータ		失敗:NULL
*/
EXTERN uint8* TextDrawRender(
	TEXT_DRAW* text_draw,
	const char* utf8_text,
	int num_character,
	int* width,
	int* height
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TEXT_H_
