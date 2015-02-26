#ifndef _INCLUDED_TEXTURE_H_
#define _INCLUDED_TEXTURE_H_

#include <GL/glew.h>
#include <GL/glut.h>
#include "types.h"

/*
 TEXTURE_BASE構造体
 テクスチャの基本データ
*/
typedef struct _TEXTURE_BASE
{
	GLuint id;		// テクスチャのID
	GLenum type;	// テクスチャのタイプ(1D or 2D or 3D)
	int width;		// テクスチャ画像の幅
	int height;		// テクスチャ画像の高さ
	int channel;	// テクスチャ画像のチャンネル数
} TEXTURE_BASE;

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
 InitializeTexture2D関数
 2Dテクスチャを生成し初期化する
 引数
 texture	: テクスチャの基本データ構造体
 pixels		: テクスチャにする画像のピクセルデータ
 width		: 画像の幅
 height		: 画像の高さ
 channel	: 画像のチャンネル数
				(グレースケール:1、グレースケール+αチャンネル:2、RGB:3、RGBA4)
*/
EXTERN void InitializeTexture2D(
	TEXTURE_BASE* texture,
	uint8* pixels,
	int width,
	int height,
	int channel
);

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

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TEXTURE_H_
