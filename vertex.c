#include <string.h>
#include "vertex.h"

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
void InitializeVertexBuffer(
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
)
{
	(void)memset(buffer, 0, sizeof(*buffer));

	// データ配置を記憶するバッファを生成
	glGenVertexArrays(1, &buffer->vertex_array);

	// 頂点バッファを生成
	glGenBuffers(1, &buffer->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// インデックスバッファを生成
	glGenBuffers(1, &buffer->index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// データの配置記憶を開始
	glBindVertexArray(buffer->vertex_array);

	// 頂点バッファの配置を設定
		// 生成した頂点バッファを呼び出す
	glBindBuffer(GL_ARRAY_BUFFER, buffer->vertex_buffer);

	// 頂点のデータ位置を設定
	glVertexAttribPointer(vertex_key, 3, GL_FLOAT, GL_FALSE, data_stride, vertex_offset);
	if(texture_offset != NULL)
	{
		glVertexAttribPointer(texture_key, 2, GL_FLOAT, GL_FALSE, data_stride, texture_offset);
	}
	if(color_offset != NULL)
	{
		glVertexAttribPointer(color_key, 4, GL_UNSIGNED_BYTE, GL_TRUE, data_stride, color_offset);
	}
	glEnableVertexAttribArray(vertex_key);
	if(texture_offset != NULL)
	{
		glEnableVertexAttribArray(texture_key);
	}
	if(color_offset != NULL)
	{
		glEnableVertexAttribArray(color_key);
	}

	// データ配置の記憶終了
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// データ配置を無効化しておく
	glDisableVertexAttribArray(vertex_key);
	if(texture_offset != NULL)
	{
		glDisableVertexAttribArray(texture_key);
	}
	if(color_offset != NULL)
	{
		glDisableVertexAttribArray(color_key);
	}
}

#ifdef __cplusplus
}
#endif
