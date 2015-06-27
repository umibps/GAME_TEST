#ifndef _INCLUDED_TEXTURE_H_
#define _INCLUDED_TEXTURE_H_

#include <GL/glew.h>
#include <GL/glut.h>
#include "types.h"
#include "text.h"
#include "utils.h"

/*
 TEXTURE_BASE構造体
 テクスチャの基本データ
*/
typedef struct _TEXTURE_BASE
{
	GLuint id;				// テクスチャのID
	char *file_path;		// 画像ファイルのパス
	GLenum type;			// テクスチャのタイプ(1D or 2D or 3D)
	int width;				// テクスチャ画像の幅
	int height;				// テクスチャ画像の高さ
	int channel;			// テクスチャ画像のチャンネル数
	int reference_count;	// 参照カウント
} TEXTURE_BASE;

/*
 TEXTURES構造体
 プログラム全体で保持しているテクスチャを管理
*/
typedef struct _IMAGE_TEXTURES
{
	STRING_HASH_TABLE hash_table;	// ハッシュテーブルでダブリ防止
} IMAGE_TEXTURES;

#ifdef __cplusplus
extern "C" {
#endif

/*
 GenerateTexgture関数
 画像のピクセルデータを渡しOpenGLのテクスチャを生成する
 引数
 pixels		: 画像のピクセルデータ
 width		: 画像の幅
 height		: 画像の高さ
 channel	: 画像のチャンネル数
				 (グレースケール:1、グレースケール+αチャンネル:2、RGB:3、RGBA4)
 返り値
 生成されたテクスチャのID
*/
EXTERN GLuint GenerateTexture(
	uint8* pixels,
	int width,
	int height,
	int channel
);

/*
 SetTextureWrap関数
 テクスチャのはみ出し部の扱いを設定する
 引数
 texture_id	: テクスチャオブジェクトのID
 wrap_mode	: 繰り返し:GL_REPEAT, クリッピング:GL_CLAMP
*/
EXTERN void SetTextureWrap(GLuint texture_id, GLenum wrap_mode);

/*
 TextureCountUpReference関数
 テクスチャの参照カウントを増やす
 引数
 texture	: 参照カウントを増やすテクスチャ
 返り値
	参照カウントを増やしたテクスチャ
*/
EXTERN TEXTURE_BASE* TextureCountUpReference(TEXTURE_BASE* texture);

/*
 InitializeTexture2D関数
 2Dテクスチャを生成し初期化する
 引数
 texture	: テクスチャの基本データ構造体
 file_path	: 画像ファイルのパス
 pixels		: テクスチャにする画像のピクセルデータ
 width		: 画像の幅
 height		: 画像の高さ
 channel	: 画像のチャンネル数
				(グレースケール:1、グレースケール+αチャンネル:2、RGB:3、RGBA4)
*/
EXTERN void InitializeTexture2D(
	TEXTURE_BASE* texture,
	uint8* pixels,
	const char* file_path,
	int width,
	int height,
	int channel
);

/*
 DeleteTexture2D関数
 2Dテクスチャを削除する
 引数
 texthre	: 削除するテクスチャ
*/
EXTERN void DeleteTexture2D(TEXTURE_BASE* texture);

/*
 InitializeImageTexture関数
 画像ファイルを開き、テクスチャを生成する
 引数
 texture	: テクスチャの基本データ構造体
 path		: ファイルパス
 open_func	: ファイルを開くのに使う関数ポインタ
 read_fucn	: ファイルの読み込みに使う関数ポインタ
 seek_func	: ファイルのシークに使う関数ポインタ
 tell_func	: ファイルのシーク位置取得に使う関数ポインタ
 close_func	: ファイルを閉じるのに使う関数ポインタ
 user_data	: ファイル操作時に使いたいユーザー定義データ
 返り値
	成功:TRUE	失敗:FALSE
*/
EXTERN int InitializeImageTexture(
	TEXTURE_BASE* texture,
	const char* path,
	void* (*open_func)(const char*, const char*, void*),
	size_t(*read_func)(void*, size_t, size_t, void*),
	int(*seek_func)(void*, long, int),
	long(*tell_func)(void*),
	int(*close_func)(void*, void*),
	void* user_data
);

/*
 InitializeTextTexture関数
 テキストのテクスチャを生成する
 引数
 texture		: テクスチャの基本データ構造体
 text_draw		: テキストの描画を管理するデータ
 utf8_text		: 描画するテキスト
 num_character	: 描画する文字数
 返り値
	成功:TRUE	失敗:FALSE
*/
EXTERN int InitializeTextTexture(
	TEXTURE_BASE* texture,
	TEXT_DRAW* text_draw,
	const char* utf8_text,
	int num_character
);

/*
 TextTextureNew関数
 テキストのテクスチャをメモリを確保して生成する
 引数
 text_draw		: テキストの描画を管理するデータ
 utf8_text		: 描画するテキスト
 num_character	: 描画する文字数
 返り値
	生成したテクスチャのデータ(不要になったらMEM_FREE_FUNC)
*/
EXTERN TEXTURE_BASE* TextTextureNew(
	TEXT_DRAW* text_draw,
	const char* utf8_text,
	int num_character
);

/*
 ImageTextureNew関数
 画像ファイルからテクスチャをダブり防止して作成する
 引数
*/
EXTERN TEXTURE_BASE* ImageTextureNew(
	const char* path,
	void* (*open_func)(const char*, const char*, void*),
	size_t(*read_func)(void*, size_t, size_t, void*),
	int(*seek_func)(void*, long, int),
	long(*tell_func)(void*),
	int(*close_func)(void*, void*),
	void* user_data,
	IMAGE_TEXTURES* textures
);

/*
 InitializeSquareTexture関数
 長方形のテクスチャを生成する
 引数
 texture	: テクスチャの基本データ構造体
 width		: 長方形の幅
 height		: 長方形の高さ
 返り値
	成功:TRUE	失敗:FALSE
*/
EXTERN int InitializeSquareTexture(
	TEXTURE_BASE* texture,
	int width,
	int height
);

/*
 SquareTextureNew関数
 長方形のテクスチャをメモリを確保して生成する
 引数
 width	: 長方形の幅
 height	: 長方形の高さ
 返り値
	生成したテクスチャのデータ(不要になったらMEM_FREE_FUNC)
*/
EXTERN TEXTURE_BASE* SquareTextureNew(int width, int height);

/*
 InitializeImageTextures関数
 テクスチャ全体を管理するデータを初期化
 引数
 textures	: テクスチャ全体を管理するデータ
*/
EXTERN void InitializeImageTextures(IMAGE_TEXTURES* textures);

/*
 ReleaseImageTextures関数
 テクスチャ全体を管理するデータを開放
 引数
 textures	: テクスチャ全体を管理するデータ
*/
EXTERN void ReleaseImageTextures(IMAGE_TEXTURES* textures);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TEXTURE_H_
