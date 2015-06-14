#ifndef _INCLUDED_DRAW_ITEM_H_
#define _INCLUDED_DRAW_ITEM_H_

#include "types.h"
#include "shader_program.h"
#include "texture.h"

/*
 DRAW_ITEM_BASE構造体
 描画アイテムの基本データ
*/
typedef struct _DRAW_ITEM_BASE
{
	unsigned int priority;							// 描画優先度
	void (*draw)(struct _DRAW_ITEM_BASE*);			// 描画時に使用する関数ポインタ
	void (*delete_func)(struct _DRAW_ITEM_BASE*);	// データ削除時に使用する関数ポインタ
} DRAW_ITEM_BASE;

/*
 DRAW_SQUARE_ITEM構造体
 画像全体を使って描画するアイテム
*/
typedef struct _DRAW_SQUARE_ITEM
{
	DRAW_ITEM_BASE base_data;
	DRAW_SQUARE_PROGRAM *program;	// 描画に使用するシェーダー
	TEXTURE_BASE *texture;			// 描画するテクスチャ
	float x, y;						// 描画する座標
	float zoom;						// 拡大・縮小率
	float rotate;					// 回転角
	uint32 color;					// 色
} DRAW_SQUARE_ITEM;

/*
 CLIP_DRAW_ITEM構造体
 画像でクリッピングして描画するアイテム
*/
typedef struct _CLIP_DRAW_ITEM
{
	DRAW_ITEM_BASE base_data;
	CLIP_DRAW_PROGRAM *program;		// 描画に使用するシェーダー
	TEXTURE_BASE *texture;			// 描画するテクスチャ
	TEXTURE_BASE *clip;				// クリッピングに使うテクスチャ
	float x, y;						// 描画する座標
	float texture_position[2][2];	// テクスチャの座標(UV, 左上と右下)
	float clip_position[2][2];		// クリッピングに使うテクスチャの座標(UV, 左上と右下)
	float zoom;						// 拡大・縮小率
	float clip_zoom;				// クリッピングに使うテクスチャの拡大・縮小率
	float rotate;					// 回転角
	float clip_rotate;				// クリッピングに使うテクスチャの回転角
	uint32 color;					// 色
} CLIP_DRAW_ITEM;

#ifdef __cplusplus
extern "C" {
#endif

/*
 DeleteDrawItemBase関数
 描画アイテムを削除する
 引数
 item	: 削除するアイテム
*/
EXTERN void DeleteDrawItemBase(DRAW_ITEM_BASE* item);

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
EXTERN void InitializeDrawSquareItem(
	DRAW_SQUARE_ITEM* item,
	TEXTURE_BASE* texture,
	float x,
	float y,
	float zoom,
	float rotate,
	uint32 color,
	DISPLAY_PROGRAMS* programs
);

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
EXTERN DRAW_ITEM_BASE* DrawSquareItemNew(
	TEXTURE_BASE* texture,
	float x,
	float y,
	float zoom,
	float rotate,
	uint32 color,
	DISPLAY_PROGRAMS* programs
);

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
EXTERN DRAW_ITEM_BASE* DrawSquareItemNew(
	TEXTURE_BASE* texture,
	float x,
	float y,
	float zoom,
	float rotate,
	uint32 color,
	DISPLAY_PROGRAMS* programs
);

/*
 InitializeClipDrawItem関数
 画像でクリッピングして描画するアイテムの初期化
 引数
 item				: 初期化するアイテム
 texture			: 描画するテクスチャ
 clip				: クリッピングに使うテクスチャ
 x					: 画像の左上のX座標
 y					: 画像の左上のY座標
 texture_position	: テクスチャの座標 (UV, 左上と右下, NULL指定可)
 clip_position		: クリッピングに使うテクスチャの座標 (UV, 左上と右下, NULL指定可)
 zoom				: 拡大・縮小率
 clip_zoom			: クリッピングに使うテクスチャの拡大・縮小率
 rotate				: 回転角
 clip_rotate		: クリッピングに使うテクスチャの回転角
 color				: 画像に適用する色(0xFFFFFFFFで元の色)
 programs			: シェーダーオブジェクトを管理するデータ
*/
EXTERN void InitializeClipDrawItem(
	CLIP_DRAW_ITEM* item,
	TEXTURE_BASE* texture,
	TEXTURE_BASE* clip,
	float x,
	float y,
	float texture_position[2][2],
	float clip_position[2][2],
	float zoom,
	float clip_zoom,
	float rotate,
	float clip_rotate,
	uint32 color,
	DISPLAY_PROGRAMS* programs
);

/*
 ClipDrawItemNew関数
 画像全体を使って描画するアイテムの作成
 引数
 texture			: 描画するテクスチャ
 clip				: クリッピングに使うテクスチャ
 x					: 画像の左上のX座標
 y					: 画像の左上のY座標
 texture_position	: テクスチャの座標 (UV, 左上と右下, NULL指定可)
 clip_position		: クリッピングに使うテクスチャの座標 (UV, 左上と右下, NULL指定可)
 zoom				: 拡大・縮小率
 clip_zoom			: クリッピングに使うテクスチャの拡大・縮小率
 rotate				: 回転角
 clip_rotate		: クリッピングに使うテクスチャの回転角
 color				: 画像に適用する色(0xFFFFFFFFで元の色)
 programs			: シェーダーオブジェクトを管理するデータ
 返り値
	作成したアイテムのアドレス(不要になったらMEM_FREE_FUNC)
*/
EXTERN DRAW_ITEM_BASE* ClipDrawItemNew(
	TEXTURE_BASE* texture,
	TEXTURE_BASE* clip,
	float x,
	float y,
	float texture_position[2][2],
	float clip_position[2][2],
	float zoom,
	float clip_zoom,
	float rotate,
	float clip_rotate,
	uint32 color,
	DISPLAY_PROGRAMS* programs
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_DRAW_ITEM_H_
