#ifndef _INCLUDED_GAME_DATA_H_
#define _INCLUDED_GAME_DATA_H_

#include "types.h"
#include "vertex.h"
#include "texture.h"
#include "display.h"

struct _GAME_DATA
{
	DISPLAY_PROGRAMS programs;
	TEXTURE_BASE texture;
	DRAW_VERTEX vertex;
	VERTEX_BUFFER vertex_buffer;
};

#ifdef __cplusplus
extern "C" {
#endif

EXTERN GAME_DATA* GetGameData(void);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_GAME_DATA_H_
