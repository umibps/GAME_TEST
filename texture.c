#include <string.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "texture.h"
#include "image_file.h"
#include "utils.h"
#include "types.h"

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
GLuint GenerateTexture(
	uint8* pixels,
	int width,
	int height,
	int channel
)
{
	GLuint texture_id;		// 生成されたテクスチャのID

	// OpenGLにテクスチャ生成を依頼してIDを貰う
	glGenTextures(1, &texture_id);

	// IDが0なら生成失敗
	if(texture_id == 0)
	{
		return 0;
	}

	// 生成したテクスチャを呼び出し
		// (テクスチャの設定とピクセルデータの引き渡しを行うため
	glBindTexture(GL_TEXTURE_2D, texture_id);

	// テクスチャを拡大・縮小したときの設定(線形補間)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// 画像のチャンネル数で処理を切り替え
	switch(channel)
	{
	case 1:	// グレースケールの画像はαチャンネルのみのテクスチャにする
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA,
			GL_UNSIGNED_BYTE, pixels);
		break;
	case 2:	// グレースケール+αチャンネルの画像はRGBAに変換する
		{
			uint8 *texture_pixels = (uint8*)MEM_ALLOC_FUNC(width*height*4);
			int i;
			for(i=0; i<width*height; i++)
			{
				texture_pixels[i*4] = pixels[i*2];
				texture_pixels[i*4+1] = pixels[i*2];
				texture_pixels[i*4+2] = pixels[i*2];
				texture_pixels[i*4+3] = pixels[i*2+1];
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, texture_pixels);

			MEM_FREE_FUNC(texture_pixels);
		}
		break;
	case 3:	// RGBの画像データはそのままテクスチャにする
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB,
			GL_UNSIGNED_BYTE, pixels);
		break;
	case 4:	// RGBAの画像データもそのままテクスチャにする
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, pixels);
		break;
	default:	// それ以外のフォーマットは未対応
		glDeleteTextures(1, &texture_id);
		return 0;
	}

	// テクスチャの呼び出しを終了
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture_id;
}

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
void InitializeTexture2D(
	TEXTURE_BASE* texture,
	uint8* pixels,
	int width,
	int height,
	int channel
)
{
	(void)memset(texture, 0, sizeof(*texture));

	texture->id = GenerateTexture(pixels, width, height, channel);
	texture->type = GL_TEXTURE_2D;
	texture->width = width;
	texture->height = height;
	texture->channel = channel;
}

/*
 ReleaseTexture2D関数
 2Dテクスチャを開放する
 引数
 texture	: テクスチャの基本データ構造体
*/
void ReleaseTexture2D(TEXTURE_BASE* texture)
{
	glDeleteTextures(1, &texture->id);
	texture->id = 0;
}

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
int InitializeImageTexture(
	TEXTURE_BASE* texture,
	const char* path,
	void* (*open_func)(const char*, const char*, void*),
	size_t(*read_func)(void*, size_t, size_t, void*),
	int(*seek_func)(void*, long, int),
	long(*tell_func)(void*),
	int(*close_func)(void*, void*),
	void* user_data
)
{
	const char *extention;	// 拡張子
	const char *str;		// 文字列の参照位置
	void *stream;			// 画像読み込み用ストリーム
	uint8 *pixels = NULL;	// 画像のピクセルデータ
	int width;				// 画像の幅
	int height;				// 画像の高さ
	int channel;			// 画像のチャンネル数

	// ファイルを開く
	stream = open_func(path, "rb", user_data);
	if(stream == NULL)
	{	// ファイルオープン失敗
		return FALSE;
	}

	// 拡張子を取得
	extention = str = path;
	while(*str != '\0')
	{
		if(*str == '.')
		{
			extention = str;
		}

		str++;
	}

	// 画像データをデコードする
	if(StringCompareIgnoreCase(extention, ".png") == 0)
	{
		pixels = ReadPngStream(stream, read_func,
			&width, &height, &channel);
	}
	else if(StringCompareIgnoreCase(extention, ".jpg") == 0
		|| StringCompareIgnoreCase(extention, ".jpeg") == 0)
	{
		long data_size;
		(void)seek_func(stream, 0, SEEK_END);
		data_size = tell_func(stream);
		(void)seek_func(stream, 0, SEEK_SET);
		pixels = ReadJpegStream(stream, read_func, data_size,
			&width, &height, &channel);
	}

	if(pixels == NULL)
	{
		(void)close_func(stream, user_data);
		return FALSE;
	}

	// OpenGLのテクスチャを生成
	InitializeTexture2D(texture, pixels, width, height, channel);

	// メモリを開放
	MEM_FREE_FUNC(pixels);
	(void)close_func(stream, user_data);

	return texture->id != 0;
}

#ifdef __cplusplus
}
#endif
