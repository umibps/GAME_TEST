#ifndef _INCLUDED_GAME_DATA_H_
#define _INCLUDED_GAME_DATA_H_

#include "types.h"
#include "vertex.h"
#include "texture.h"
#include "display.h"
#include "text.h"

/*
 GAME_DATA構造体
 ゲーム全体のデータを管理
*/
struct _GAME_DATA
{
	DISPLAY_PROGRAMS programs;
	TEXTURE_BASE texture1, texture2, texture3;
	DRAW_VERTEX vertex;
	VERTEX_BUFFER vertex_buffer;
	TEXT_DRAW text_draw;
};

#ifdef __cplusplus
extern "C" {
#endif

EXTERN GAME_DATA* GetGameData(void);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_GAME_DATA_H_
