#include <string.h>
#include "draw_item.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 DeleteDrawItemBase関数
 描画アイテムを削除する
 引数
 item	: 削除するアイテム
*/
void DeleteDrawItemBase(DRAW_ITEM_BASE* item)
{
	if(item->delete_func != NULL)
	{
		item->delete_func(item);
	}
	MEM_FREE_FUNC(item);
}

/*
 DrawSquare関数
 画像全体を使って描画する
 引数
 item	: 描画するアイテム
*/
static void DrawSquare(DRAW_SQUARE_ITEM* item)
{
	// 使用するシェーダーオブジェクトを決定
	glUseProgram(item->program->base_data.program_id);

	// 使用するテクスチャの設定
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, item->texture->id);
	glUniform1i(item->program->texture_uniform_location, 0);

	// 使用する頂点配列を決定
	glBindVertexArray(item->program->vertex_buffer.vertex_array);

	// uniform属性の値を設定
	glUniform2f(item->program->position_uniform_location,
		item->x, item->y);
	glUniform2f(item->program->texture_size_uniform_location,
		(GLfloat)item->texture->width, (GLfloat)item->texture->height);
	glUniform1f(item->program->zoom_uniform_location, item->zoom);
	glUniform1f(item->program->rotate_uniform_location, item->rotate);
	glUniform4f(item->program->color_uniform_location,
		((item->color & 0xFF000000) >> 24) * DIV_PIXEL,
		((item->color & 0x00FF0000) >> 16) * DIV_PIXEL,
		((item->color & 0x0000FF00) >>  8) * DIV_PIXEL,
		(item->color & 0x000000FF) * DIV_PIXEL
	);

	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, NULL);

	glBindVertexArray(0);

	glUseProgram(0);
}

/*
 DeleteDrawSquareItem関数
 画像全体を使って描画するアイテムの削除
 引数
 item	: 削除するアイテム
*/
static void DeleteDrawSquareItem(DRAW_SQUARE_ITEM* item)
{
	item->texture->reference_count--;
	MEM_FREE_FUNC(item);
}

/*
 InitializeDrawSquareItem関数
 画像全体を使って描画するアイテムの初期化
 引数
 item		: 初期化するアイテム
 texture	: 描画するテクスチャ
 x			: 画像の左上のX座標
 y			: 画像の左上のY座標
 zoom		: 拡大・縮小率
 rotate		: 回転角
 color		: 画像に適用する色(0xFFFFFFFFで元の色)
 programs	: シェーダーオブジェクトを管理するデータ
*/
void InitializeDrawSquareItem(
	DRAW_SQUARE_ITEM* item,
	TEXTURE_BASE* texture,
	float x,
	float y,
	float zoom,
	float rotate,
	uint32 color,
	DISPLAY_PROGRAMS* programs
)
{
	(void)memset(item, 0, sizeof(*item));

	item->texture = texture;
	item->x = x;
	item->y = y;
	item->zoom = zoom;
	item->rotate = rotate;
	item->color = color;
	item->program = &programs->draw_square;

	item->base_data.draw = (void(*)(DRAW_ITEM_BASE*))DrawSquare;
	item->base_data.delete_func = (void(*)(DRAW_ITEM_BASE*))DeleteDrawSquareItem;
}

/*
 DrawSquareItemNew関数
 画像全体を使って描画するアイテムの作成
 引数
 texture	: 描画するテクスチャ
 x			: 画像の左上のX座標
 y			: 画像の左上のY座標
 zoom		: 拡大・縮小率
 rotate		: 回転角
 color		: 画像に適用する色(0xFFFFFFFFで元の色)
 programs	: シェーダーオブジェクトを管理するデータ
 返り値
	作成したアイテムのアドレス(不要になったらMEM_FREE_FUNC)
*/
DRAW_ITEM_BASE* DrawSquareItemNew(
	TEXTURE_BASE* texture,
	float x,
	float y,
	float zoom,
	float rotate,
	uint32 color,
	DISPLAY_PROGRAMS* programs
)
{
	DRAW_SQUARE_ITEM *item = (DRAW_SQUARE_ITEM*)MEM_ALLOC_FUNC(sizeof(*item));

	InitializeDrawSquareItem(item, texture, x, y, zoom, rotate, color, programs);

	return (DRAW_ITEM_BASE*)item;
}

#ifdef __cplusplus
}
#endif