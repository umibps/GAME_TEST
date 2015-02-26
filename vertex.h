#ifndef _INCLUDED_VERTEX_H_
#define _INCLUDED_VERTEX_H_

#include <stdarg.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "types.h"

/*
 VERTEX_BUFFER構造体
 描画時のOpenGLのバッファを管理
*/
typedef struct _VERTEX_BUFFER
{
	GLuint vertex_array;	// データ配置を記憶するバッファ
	GLuint vertex_buffer;	// 頂点バッファオブジェクト
	GLuint index_buffer;	// インデックスバッファオブジェクト
} VERTEX_BUFFER;

/*
 DRAW_VERTEX構造体
 描画時の頂点データの構造
*/
typedef struct _DRAW_VERTEX
{
	float position[3];		// 描画する座標(XYZ)
	float texture_coord[2];	// 貼り付けるテクスチャの座標(UV)
	uint8 color[4];			// 設定する色(RGBA)
} DRAW_VERTEX;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeVertexBuffer関数
 頂点バッファを初期化する
 引数
 buffer			: 頂点バッファを管理する構造体
 vertex_size	: 頂点バッファのサイズ
 index_size		: インデックスバッファのサイズ
 data_stride	: 頂点データ構造体のバイト数
 可変長引数		: 配列の長さ(int), データのID(int), データの型(GLenum),
					データの正規化有無(GL_TRUE or GL_FALSE), データのオフセット(size_t)
					で指定する (最後は0)
*/
EXTERN void InitializeVertexBuffer(
	VERTEX_BUFFER* buffer,
	size_t vertex_size,
	size_t index_size,
	size_t data_stride,
	int first_size,
	...
);

/*
 InitializeVertexArray関数
 頂点バッファを使用しない場合のデータ配置を設定する
 引数
 vertex_array	: データ配置を記憶する頂点配列オブジェクト
 vertex_size	: 頂点バッファのサイズ
 index_size		: インデックスバッファのサイズ
 data_stride	: 頂点データ構造体のバイト数
 可変長引数		: 配列の長さ(int), データのID(int), データの型(GLenum),
					データの正規化有無(GL_TRUE or GL_FALSE), データのオフセット(size_t)
					で指定する (最後は0)
*/
EXTERN void InitializeVertexArray(
	GLuint* vertex_array,
	size_t data_stride,
	int first_size,
	...
);

/*
 SetAttributePointer関数
 OpenGLに渡す頂点データの配置を設定する
 引数
 stride		: 頂点データ1つ分のバイト数
 可変長引数	: 配列の長さ(int), データのID(int), データの型(GLenum),
				データの正規化有無(GL_TRUE or GL_FALSE), データのオフセット(size_t)
				で指定する (最後は0)
*/
EXTERN void SetAttributePointer(
	size_t stride,
	int first_size,
	...
);

/*
 VaListAttributePointer関数
 va_listからOpenGLに渡すデータの配置を設定する
 引数
 stride		: 頂点データ1つ分のバイト数
 first_size	: 最初のデータの配列長
 list		: va_list(内容はSetAttributePointer関数を参照)
*/
EXTERN void VaListAttributePointer(va_list list, int first_size, size_t stride);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_VERTEX_H_
