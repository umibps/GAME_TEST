#ifndef _INCLUDED_DISPLAY_H_
#define _INCLUDED_DISPLAY_H_

#include "types.h"
#include "shader_program.h"
#include "draw_item.h"
#include "texture.h"
#include "utils.h"

/*
 DISPLAY_DATA構造体
 描画に関連するデータ
*/
typedef struct _DISPLAY_DATA
{
	DISPLAY_PROGRAMS programs;	// シェーダープログラム
	IMAGE_TEXTURES textures;	// テクスチャ
	PRIORITY_ARRAY draw_items;	// 描画するアイテム
	TEXT_DRAW text_draw;		// テキスト描画用データ
	int display_width;			// 描画領域の幅
	int display_height;			// 描画領域の高さ
} DISPLAY_DATA;

#ifdef __cplusplus
extern "C" {
#endif

EXTERN void Display(void);

/*
 ReshapeWindow関数
 ウィンドウにサイズが変更された時のコールバック関数
 引数
 width	: OpenGLが描画する領域の幅
 height	: OpenGLが描画する領域の高さ
*/
EXTERN void ReshapeWindow(int width, int height);

/*
 InitializeDisplayData関数
 描画に関連するデータを初期化する
 引数
 display_data	: 描画に関連するデータ
*/
EXTERN void InitializeDisplayData(DISPLAY_DATA* display_data);

/*
 SetUpStateOfOpenGL関数
 OpenGLの状態をデフォルト状態に設定する
 引数
 game_data	: ゲーム全体を管理するデータ
*/
EXTERN void SetUpStateOfOpenGL(GAME_DATA* game_data);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_DISPLAY_H_
