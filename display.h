#ifndef _INCLUDED_DISPLAY_H_
#define _INCLUDED_DISPLAY_H_

#include "types.h"
#include "shader_program.h"
#include "draw_item.h"
#include "texture.h"
#include "utils.h"

/*
 DISPLAY_DATA�\����
 �`��Ɋ֘A����f�[�^
*/
typedef struct _DISPLAY_DATA
{
	DISPLAY_PROGRAMS programs;	// �V�F�[�_�[�v���O����
	IMAGE_TEXTURES textures;	// �e�N�X�`��
	PRIORITY_ARRAY draw_items;	// �`�悷��A�C�e��
	TEXT_DRAW text_draw;		// �e�L�X�g�`��p�f�[�^
	int display_width;			// �`��̈�̕�
	int display_height;			// �`��̈�̍���
} DISPLAY_DATA;

#ifdef __cplusplus
extern "C" {
#endif

EXTERN void Display(void);

/*
 ReshapeWindow�֐�
 �E�B���h�E�ɃT�C�Y���ύX���ꂽ���̃R�[���o�b�N�֐�
 ����
 width	: OpenGL���`�悷��̈�̕�
 height	: OpenGL���`�悷��̈�̍���
*/
EXTERN void ReshapeWindow(int width, int height);

/*
 InitializeDisplayData�֐�
 �`��Ɋ֘A����f�[�^������������
 ����
 display_data	: �`��Ɋ֘A����f�[�^
*/
EXTERN void InitializeDisplayData(DISPLAY_DATA* display_data);

/*
 SetUpStateOfOpenGL�֐�
 OpenGL�̏�Ԃ��f�t�H���g��Ԃɐݒ肷��
 ����
 game_data	: �Q�[���S�̂��Ǘ�����f�[�^
*/
EXTERN void SetUpStateOfOpenGL(GAME_DATA* game_data);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_DISPLAY_H_
