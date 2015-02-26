#ifndef _INCLUDED_VERTEX_H_
#define _INCLUDED_VERTEX_H_

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
 vertex_key		: GLSLに渡した頂点座標の識別ID
 vertex_offset	: 頂点座標データの位置
 texture_key	: GLSLに渡したテクスチャ座標の識別ID
 texture_offset	: テクスチャ座標データの位置
 color_key		: GLSLに渡した色データの識別ID
 color_offset	: 色データの位置
*/
EXTERN void InitializeVertexBuffer(
	VERTEX_BUFFER* buffer,
	size_t vertex_size,
	size_t index_size,
	size_t data_stride,
	int vertex_key,
	void* vertex_offset,
	int texture_key,
	void* texture_offset,
	int color_key,
	void* color_offset
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_VERTEX_H_
