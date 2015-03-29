#ifndef _INCLUDED_GAME_DATA_H_
#define _INCLUDED_GAME_DATA_H_

#include "types.h"
#include "vertex.h"
#include "texture.h"
#include "display.h"
#include "text.h"
#include "draw_item.h"
#include "input.h"
#include "task.h"
#include "utils.h"

/*
 GAME_DATA構造体
 ゲーム全体のデータを管理
*/
struct _GAME_DATA
{
	DISPLAY_DATA display_data;	// 描画用のデータ
	TASKS tasks;				// タスク実行用のデータ
	GAME_INPUT input;			// 入力処理用のデータ
};

#ifdef __cplusplus
extern "C" {
#endif

EXTERN GAME_DATA* GetGameData(void);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_GAME_DATA_H_
