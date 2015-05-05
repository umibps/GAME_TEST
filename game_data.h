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
#include "sound.h"

/*
 GAME_DATA�\����
 �Q�[���S�̂̃f�[�^���Ǘ�
*/
struct _GAME_DATA
{
	DISPLAY_DATA display_data;		// �`��p�̃f�[�^
	TASKS tasks;					// �^�X�N���s�p�̃f�[�^
	GAME_INPUT input;				// ���͏����p�̃f�[�^
	SOUND_CONTEXT sound_context;	// �����Đ��p�̊�{�I�ȃf�[�^
	SOUNDS sounds;					// BGM, ���ʉ��S�̂̃f�[�^
};

#ifdef __cplusplus
extern "C" {
#endif

EXTERN GAME_DATA* GetGameData(void);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_GAME_DATA_H_
