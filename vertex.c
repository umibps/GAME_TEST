#include <string.h>
<<<<<<< HEAD
#include <stdarg.h>
=======
>>>>>>> 595175bcef89dcfa5ca4a1ef8275fb7d26b0dc89
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
<<<<<<< HEAD
 可変長引数		: 配列の長さ(int), データのID(int), データの型(GLenum),
					データの正規化有無(GL_TRUE or GL_FALSE), データのオフセット(size_t)
					で指定する (最後は0)
=======
 vertex_key		: GLSLに渡した頂点座標の識別ID
 vertex_offset	: 頂点座標データの位置
 texture_key	: GLSLに渡したテクスチャ座標の識別ID
 texture_offset	: テクスチャ座標データの位置
 color_key		: GLSLに渡した色データの識別ID
 color_offset	: 色データの位置
>>>>>>> 595175bcef89dcfa5ca4a1ef8275fb7d26b0dc89
*/
void InitializeVertexBuffer(
	VERTEX_BUFFER* buffer,
	size_t vertex_size,
	size_t index_size,
	size_t data_stride,
<<<<<<< HEAD
	int first_size,
	...
)
{
	int key;				// データのID
	int size;				// データの配列長
	GLenum type;			// データの型
	GLboolean normalized;	// 正規化有無
	size_t offset;			// データの位置
	va_list list;			// 可変長引数のリスト

=======
	int vertex_key,
	void* vertex_offset,
	int texture_key,
	void* texture_offset,
	int color_key,
	void* color_offset
)
{
>>>>>>> 595175bcef89dcfa5ca4a1ef8275fb7d26b0dc89
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
<<<<<<< HEAD
	va_start(list, first_size);
	VaListAttributePointer(list, first_size, data_stride);
	va_end(list);

	// データ配置の記憶終了
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// データ配置を無効化しておく
	va_start(list, first_size);
	size = first_size;
	while(size > 0)
	{
		key = va_arg(list, int);
		type = va_arg(list, GLenum);
		normalized = va_arg(list, GLboolean);
		offset = va_arg(list, size_t);
		glDisableVertexAttribArray(key);
		size = va_arg(list, int);
	}
	va_end(list);
}

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
void InitializeVertexArray(
	GLuint* vertex_array,
	size_t data_stride,
	int first_size,
	...
)
{
	int key;				// データのID
	int size;				// データの配列長
	GLenum type;			// データの型
	GLboolean normalized;	// 正規化有無
	size_t offset;			// データの位置
	va_list list;			// 可変長引数のリスト

	// 頂点配列オブジェクトを作成
	glGenVertexArrays(1, vertex_array);

	// データ配置の記憶開始
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(*vertex_array);

	// 頂点のデータ位置を設定
	va_start(list, first_size);
	VaListAttributePointer(list, first_size, data_stride);
	va_end(list);

	// データ配置の記憶終了
	glBindVertexArray(0);

	// データ配置を無効化しておく
	va_start(list, first_size);
	size = first_size;
	while(size > 0)
	{
		key = va_arg(list, int);
		type = va_arg(list, GLenum);
		normalized = va_arg(list, GLboolean);
		offset = va_arg(list, size_t);
		glDisableVertexAttribArray(key);
		size = va_arg(list, int);
	}
	va_end(list);
}

/*
 VaListAttributePointer関数
 va_listからOpenGLに渡すデータの配置を設定する
 引数
 list		: va_list(内容はSetAttributePointer関数を参照)
 stride		: 頂点データ1つ分のバイト数
 first_size	: 最初のデータの配列長
*/
void VaListAttributePointer(va_list list, int first_size, size_t stride)
{
	int key;				// データのID
	int size;				// データの配列長
	GLenum type;			// データの型
	GLboolean normalized;	// 正規化有無
	size_t offset;			// データの位置

	// 渡された引数を使いデータの配置を設定
	size = first_size;
	while(size > 0)
	{
		key = va_arg(list, int);
		type = va_arg(list, GLenum);
		normalized = va_arg(list, GLboolean);
		offset = va_arg(list, size_t);
		glVertexAttribPointer(key, size, type,
			normalized, stride, (const void*)offset);
		glEnableVertexAttribArray(key);
		size = va_arg(list, int);
	}
}

/*
 SetAttributePointer関数
 OpenGLに渡す頂点データの配置を設定する
 引数
 stride		: 頂点データ1つ分のバイト数
 可変長引数	: 配列の長さ(int), データのID(int), データの型(GLenum),
				データの正規化有無(GL_TRUE or GL_FALSE), データのオフセット(size_t)
				で指定する (最後は0)
*/
void SetAttributePointer(
	size_t stride,
	int first_size,
	...
)
{
	va_list list;

	va_start(list, first_size);
	
	VaListAttributePointer(list, first_size, stride);

	va_end(list);
}

=======
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

>>>>>>> 595175bcef89dcfa5ca4a1ef8275fb7d26b0dc89
#ifdef __cplusplus
}
#endif
