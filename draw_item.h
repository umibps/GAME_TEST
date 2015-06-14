#ifndef _INCLUDED_DRAW_ITEM_H_
#define _INCLUDED_DRAW_ITEM_H_

#include "types.h"
#include "shader_program.h"
#include "texture.h"

/*
 DRAW_ITEM_BASE�\����
 �`��A�C�e���̊�{�f�[�^
*/
typedef struct _DRAW_ITEM_BASE
{
	unsigned int priority;							// �`��D��x
	void (*draw)(struct _DRAW_ITEM_BASE*);			// �`�掞�Ɏg�p����֐��|�C���^
	void (*delete_func)(struct _DRAW_ITEM_BASE*);	// �f�[�^�폜���Ɏg�p����֐��|�C���^
} DRAW_ITEM_BASE;

/*
 DRAW_SQUARE_ITEM�\����
 �摜�S�̂��g���ĕ`�悷��A�C�e��
*/
typedef struct _DRAW_SQUARE_ITEM
{
	DRAW_ITEM_BASE base_data;
	DRAW_SQUARE_PROGRAM *program;	// �`��Ɏg�p����V�F�[�_�[
	TEXTURE_BASE *texture;			// �`�悷��e�N�X�`��
	float x, y;						// �`�悷����W
	float zoom;						// �g��E�k����
	float rotate;					// ��]�p
	uint32 color;					// �F
} DRAW_SQUARE_ITEM;

/*
 CLIP_DRAW_ITEM�\����
 �摜�ŃN���b�s���O���ĕ`�悷��A�C�e��
*/
typedef struct _CLIP_DRAW_ITEM
{
	DRAW_ITEM_BASE base_data;
	CLIP_DRAW_PROGRAM *program;		// �`��Ɏg�p����V�F�[�_�[
	TEXTURE_BASE *texture;			// �`�悷��e�N�X�`��
	TEXTURE_BASE *clip;				// �N���b�s���O�Ɏg���e�N�X�`��
	float x, y;						// �`�悷����W
	float texture_position[2][2];	// �e�N�X�`���̍��W(UV, ����ƉE��)
	float clip_position[2][2];		// �N���b�s���O�Ɏg���e�N�X�`���̍��W(UV, ����ƉE��)
	float zoom;						// �g��E�k����
	float clip_zoom;				// �N���b�s���O�Ɏg���e�N�X�`���̊g��E�k����
	float rotate;					// ��]�p
	float clip_rotate;				// �N���b�s���O�Ɏg���e�N�X�`���̉�]�p
	uint32 color;					// �F
} CLIP_DRAW_ITEM;

#ifdef __cplusplus
extern "C" {
#endif

/*
 DeleteDrawItemBase�֐�
 �`��A�C�e�����폜����
 ����
 item	: �폜����A�C�e��
*/
EXTERN void DeleteDrawItemBase(DRAW_ITEM_BASE* item);

/*
 InitializeDrawSquareItem�֐�
 �摜�S�̂��g���ĕ`�悷��A�C�e���̏�����
 ����
 item		: ����������A�C�e��
 texture	: �`�悷��e�N�X�`��
 x			: �摜�̍����X���W
 y			: �摜�̍����Y���W
 zoom		: �g��E�k����
 rotate		: ��]�p
 color		: �摜�ɓK�p����F(0xFFFFFFFF�Ō��̐F)
 programs	: �V�F�[�_�[�I�u�W�F�N�g���Ǘ�����f�[�^
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
 DrawSquareItemNew�֐�
 �摜�S�̂��g���ĕ`�悷��A�C�e���̍쐬
 ����
 texture	: �`�悷��e�N�X�`��
 x			: �摜�̍����X���W
 y			: �摜�̍����Y���W
 zoom		: �g��E�k����
 rotate		: ��]�p
 color		: �摜�ɓK�p����F(0xFFFFFFFF�Ō��̐F)
 programs	: �V�F�[�_�[�I�u�W�F�N�g���Ǘ�����f�[�^
 �Ԃ�l
	�쐬�����A�C�e���̃A�h���X(�s�v�ɂȂ�����MEM_FREE_FUNC)
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
 DrawSquareItemNew�֐�
 �摜�S�̂��g���ĕ`�悷��A�C�e���̍쐬
 ����
 texture	: �`�悷��e�N�X�`��
 x			: �摜�̍����X���W
 y			: �摜�̍����Y���W
 zoom		: �g��E�k����
 rotate		: ��]�p
 color		: �摜�ɓK�p����F(0xFFFFFFFF�Ō��̐F)
 programs	: �V�F�[�_�[�I�u�W�F�N�g���Ǘ�����f�[�^
 �Ԃ�l
	�쐬�����A�C�e���̃A�h���X(�s�v�ɂȂ�����MEM_FREE_FUNC)
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
 InitializeClipDrawItem�֐�
 �摜�ŃN���b�s���O���ĕ`�悷��A�C�e���̏�����
 ����
 item				: ����������A�C�e��
 texture			: �`�悷��e�N�X�`��
 clip				: �N���b�s���O�Ɏg���e�N�X�`��
 x					: �摜�̍����X���W
 y					: �摜�̍����Y���W
 texture_position	: �e�N�X�`���̍��W (UV, ����ƉE��, NULL�w���)
 clip_position		: �N���b�s���O�Ɏg���e�N�X�`���̍��W (UV, ����ƉE��, NULL�w���)
 zoom				: �g��E�k����
 clip_zoom			: �N���b�s���O�Ɏg���e�N�X�`���̊g��E�k����
 rotate				: ��]�p
 clip_rotate		: �N���b�s���O�Ɏg���e�N�X�`���̉�]�p
 color				: �摜�ɓK�p����F(0xFFFFFFFF�Ō��̐F)
 programs			: �V�F�[�_�[�I�u�W�F�N�g���Ǘ�����f�[�^
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
 ClipDrawItemNew�֐�
 �摜�S�̂��g���ĕ`�悷��A�C�e���̍쐬
 ����
 texture			: �`�悷��e�N�X�`��
 clip				: �N���b�s���O�Ɏg���e�N�X�`��
 x					: �摜�̍����X���W
 y					: �摜�̍����Y���W
 texture_position	: �e�N�X�`���̍��W (UV, ����ƉE��, NULL�w���)
 clip_position		: �N���b�s���O�Ɏg���e�N�X�`���̍��W (UV, ����ƉE��, NULL�w���)
 zoom				: �g��E�k����
 clip_zoom			: �N���b�s���O�Ɏg���e�N�X�`���̊g��E�k����
 rotate				: ��]�p
 clip_rotate		: �N���b�s���O�Ɏg���e�N�X�`���̉�]�p
 color				: �摜�ɓK�p����F(0xFFFFFFFF�Ō��̐F)
 programs			: �V�F�[�_�[�I�u�W�F�N�g���Ǘ�����f�[�^
 �Ԃ�l
	�쐬�����A�C�e���̃A�h���X(�s�v�ɂȂ�����MEM_FREE_FUNC)
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
