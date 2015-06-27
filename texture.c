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
		// (テクスチャの設定とピクセルデータの引き渡しを行うため)
	glBindTexture(GL_TEXTURE_2D, texture_id);

	// テクスチャを拡大・縮小したときの設定(線形補間)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// 指定されたテクスチャ座標が0～1をはみ出した際の描画方法(クリッピング)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// 画像のチャンネル数で処理を切り替え
	switch(channel)
	{
	case 1:	// グレースケールの画像はRGBAに変換する(色は255)
		{
			uint8 *texture_pixels = (uint8*)MEM_ALLOC_FUNC(width*height*4);
			int i;
			for(i=0; i<width*height; i++)
			{
				texture_pixels[i*4] = texture_pixels[i*4+1] =
					texture_pixels[i*4+2] = (pixels[i] > 0) ? 0xFF : 0x00;
				texture_pixels[i*4+3] = pixels[i];
			}
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, texture_pixels);
			MEM_FREE_FUNC(texture_pixels);
		}
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
 SetTextureWrap関数
 テクスチャのはみ出し部の扱いを設定する
 引数
 texture_id	: テクスチャオブジェクトのID
 wrap_mode	: 繰り返し:GL_REPEAT, クリッピング:GL_CLAMP
*/
void SetTextureWrap(GLuint texture_id, GLenum wrap_mode)
{
	// 指定されたテクスチャを呼び出し
		// (テクスチャの設定を行うため)
	glBindTexture(GL_TEXTURE_2D, texture_id);

	// 設定を実行
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
	
	// テクスチャの呼び出しを終了
	glBindTexture(GL_TEXTURE_2D, 0);
}

/*
 TextureCountUpReference関数
 テクスチャの参照カウントを増やす
 引数
 texture	: 参照カウントを増やすテクスチャ
 返り値
	参照カウントを増やしたテクスチャ
*/
TEXTURE_BASE* TextureCountUpReference(TEXTURE_BASE* texture)
{
	texture->reference_count++;
	return texture;
}

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
void InitializeTexture2D(
	TEXTURE_BASE* texture,
	uint8* pixels,
	const char* file_path,
	int width,
	int height,
	int channel
)
{
	(void)memset(texture, 0, sizeof(*texture));

	texture->id = GenerateTexture(pixels, width, height, channel);
	if(file_path != 0)
	{
		texture->file_path = MEM_STRDUP_FUNC(file_path);
	}
	texture->type = GL_TEXTURE_2D;
	texture->width = width;
	texture->height = height;
	texture->channel = channel;
	texture->reference_count = 1;
}

/*
 ReleaseTexture2D関数
 2Dテクスチャを開放する
 引数
 texture	: テクスチャの基本データ構造体
*/
void ReleaseTexture2D(TEXTURE_BASE* texture)
{
	texture->reference_count--;
	if(texture->file_path == NULL
		|| texture->reference_count <= 0)
	{
		glDeleteTextures(1, &texture->id);
		texture->id = 0;
		texture->reference_count = 0;
	}
}

/*
 DeleteTexture2D関数
 2Dテクスチャを削除する
 引数
 texthre	: 削除するテクスチャ
*/
void DeleteTexture2D(TEXTURE_BASE* texture)
{
	ReleaseTexture2D(texture);
	if(texture->id == 0)
	{
		MEM_FREE_FUNC(texture);
	}
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
	InitializeTexture2D(texture, pixels, path, width, height, channel);

	// メモリを開放
	MEM_FREE_FUNC(pixels);
	(void)close_func(stream, user_data);

	return texture->id != 0;
}

/*
 ImageTextureNew関数
 画像ファイルからテクスチャをダブり防止して作成する
 引数
 path		: 画像ファイルのパス
 open_func	: ファイルを開く為に使う関数ポインタ
 read_func	: データの読み込みに使う関数ポインタ
 seek_func	: データのシークに使う関数ポインタ
 tell_func	: データのシーク位置取得用の関数ポインタ
 close_func	: ファイルを閉じる為に使う関数ポインタ
 user_data	: ファイルを開く際に使う外部データ
 textures	: テクスチャ全体を管理するデータ
 返り値
	生成したテクスチャのデータ(不要になったらMEM_FREE_FUNC)
*/
TEXTURE_BASE* ImageTextureNew(
	const char* path,
	void* (*open_func)(const char*, const char*, void*),
	size_t(*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	int (*close_func)(void*, void*),
	void* user_data,
	IMAGE_TEXTURES* textures
)
{
	TEXTURE_BASE *texture;

	if((texture = StringHashTableGet(&textures->hash_table, path)) != NULL)
	{
		texture->reference_count++;
		return texture;
	}

	texture = (TEXTURE_BASE*)MEM_ALLOC_FUNC(sizeof(*texture));
	InitializeImageTexture(texture, path, open_func, read_func,
		seek_func, tell_func, close_func, user_data);
	StringHashTableAppend(&textures->hash_table,
		path, texture);

	return texture;
}

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
int InitializeTextTexture(
	TEXTURE_BASE* texture,
	TEXT_DRAW* text_draw,
	const char* utf8_text,
	int num_character
)
{
	uint8 *pixels;	// 描画結果のピクセルデータ
	int width;		// 描画結果の幅
	int height;		// 描画結果の高さ

	pixels = TextDrawRender(text_draw, utf8_text, num_character,
		&width, &height);
	if(pixels != NULL)
	{
		InitializeTexture2D(texture, pixels, NULL, width, height, 1);
		MEM_FREE_FUNC(pixels);
		return texture->id != 0;
	}

	return FALSE;
}

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
TEXTURE_BASE* TextTextureNew(
	TEXT_DRAW* text_draw,
	const char* utf8_text,
	int num_character
)
{
	TEXTURE_BASE *texture = (TEXTURE_BASE*)MEM_ALLOC_FUNC(sizeof(*texture));

	if(InitializeTextTexture(texture, text_draw, utf8_text, num_character) == FALSE)
	{
		MEM_FREE_FUNC(texture);
		return NULL;
	}

	return texture;
}

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
int InitializeSquareTexture(
	TEXTURE_BASE* texture,
	int width,
	int height
)
{
	uint8 *pixels;

	if((pixels = (uint8*)MEM_ALLOC_FUNC(width * height * 4)) != NULL)
	{
		(void)memset(pixels, 0xFF, width * height * 4);
		InitializeTexture2D(texture, pixels, NULL, width, height, 4);
		MEM_FREE_FUNC(pixels);
		return texture->id != 0;
	}

	return FALSE;
}

/*
 SquareTextureNew関数
 長方形のテクスチャをメモリを確保して生成する
 引数
 width	: 長方形の幅
 height	: 長方形の高さ
 返り値
	生成したテクスチャのデータ(不要になったらMEM_FREE_FUNC)
*/
TEXTURE_BASE* SquareTextureNew(int width, int height)
{
	TEXTURE_BASE *texture = (TEXTURE_BASE*)MEM_ALLOC_FUNC(sizeof(*texture));

	if(InitializeSquareTexture(texture, width, height) == FALSE)
	{
		MEM_FREE_FUNC(texture);
		return NULL;
	}

	return texture;
}

/*
 InitializeImageTextures関数
 テクスチャ全体を管理するデータを初期化
 引数
 textures	: テクスチャ全体を管理するデータ
*/
void InitializeImageTextures(IMAGE_TEXTURES* textures)
{
	InitializeStringHashTable(&textures->hash_table, 1024,
		(void(*)(void*))DeleteTexture2D);
}

/*
 ReleaseImageTextures関数
 テクスチャ全体を管理するデータを開放
 引数
 textures	: テクスチャ全体を管理するデータ
*/
void ReleaseImageTextures(IMAGE_TEXTURES* textures)
{
	ReleaseStringHashTable(&textures->hash_table);
}

#ifdef __cplusplus
}
#endif
