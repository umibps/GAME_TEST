#include <string.h>
#include "draw_item.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 DeleteDrawItemBase�֐�
 �`��A�C�e�����폜����
 ����
 item	: �폜����A�C�e��
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
 DrawSquare�֐�
 �摜�S�̂��g���ĕ`�悷��
 ����
 item	: �`�悷��A�C�e��
*/
static void DrawSquare(DRAW_SQUARE_ITEM* item)
{
	// �g�p����V�F�[�_�[�I�u�W�F�N�g������
	glUseProgram(item->program->base_data.program_id);

	// �g�p����e�N�X�`���̐ݒ�
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, item->texture->id);
	glUniform1i(item->program->texture_uniform_location, 0);

	// �g�p���钸�_�z�������
	glBindVertexArray(item->program->vertex_buffer.vertex_array);

	// uniform�����̒l��ݒ�
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
 DeleteDrawSquareItem�֐�
 �摜�S�̂��g���ĕ`�悷��A�C�e���̍폜
 ����
 item	: �폜����A�C�e��
*/
static void DeleteDrawSquareItem(DRAW_SQUARE_ITEM* item)
{
	DeleteTexture2D(item->texture);
	MEM_FREE_FUNC(item);
}

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

/*
 ClipDraw�֐�
 �摜�ŃN���b�s���O���ĕ`�悷��
 ����
 item	: �`�悷��A�C�e��
*/
static void ClipDraw(CLIP_DRAW_ITEM* item)
{
	// �g�p����V�F�[�_�[�I�u�W�F�N�g������
	glUseProgram(item->program->base_data.program_id);

	// �g�p����e�N�X�`���̐ݒ�
		// �`�悷��e�N�X�`��
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, item->texture->id);
	glUniform1i(item->program->texture_uniform_location, 0);
		// �N���b�s���O�Ɏg���e�N�X�`��
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, item->clip->id);
	glUniform1i(item->program->clip_texture_uniform_location, 1);

	// �g�p���钸�_�z�������
	glBindVertexArray(item->program->vertex_buffer.vertex_array);

	// uniform�����̒l��ݒ�
	glUniform2f(item->program->position_uniform_location,
		item->x, item->y);
	glUniform4f(item->program->texture_position_uniform_location,
		item->texture_position[0][0], item->texture_position[0][1], item->texture_position[1][0], item->texture_position[1][1]);
	glUniform4f(item->program->clip_position_uniform_location,
		item->clip_position[0][0], item->clip_position[0][1], item->clip_position[1][0], item->clip_position[1][1]);
	glUniform2f(item->program->texture_size_uniform_location,
		(GLfloat)item->texture->width, (GLfloat)item->texture->height);
	glUniform2f(item->program->clip_size_uniform_location,
		(GLfloat)item->clip->width, (GLfloat)item->clip->height);
	glUniform1f(item->program->zoom_uniform_location, item->zoom);
	glUniform1f(item->program->clip_zoom_uniform_location, item->clip_zoom);
	glUniform1f(item->program->rotate_uniform_location, item->rotate);
	glUniform1f(item->program->clip_rotate_uniform_location, item->clip_rotate);
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
 DeleteClipDrawItem�֐�
 �摜�ŃN���b�s���O���ĕ`�悷��A�C�e���̍폜
 ����
 item	: �폜����A�C�e��
*/
static void DeleteClipDrawItem(CLIP_DRAW_ITEM* item)
{
	DeleteTexture2D(item->texture);
	DeleteTexture2D(item->clip);
	MEM_FREE_FUNC(item);
}

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
void InitializeClipDrawItem(
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
)
{
	(void)memset(item, 0, sizeof(*item));

	item->texture = texture;
	item->clip = clip;
	item->x = x;
	item->y = y;
	if(texture_position == NULL)
	{
		item->texture_position[0][0] = 0;
		item->texture_position[0][1] = 0;
		item->texture_position[1][0] = (float)item->texture->width;
		item->texture_position[1][1] = (float)item->texture->height;
	}
	else
	{
		item->texture_position[0][0] = texture_position[0][0];
		item->texture_position[0][1] = texture_position[0][1];
		item->texture_position[1][0] = texture_position[1][0];
		item->texture_position[1][1] = texture_position[1][1];
	}
	if(clip_position == NULL)
	{
		item->clip_position[0][0] = 0;
		item->clip_position[0][1] = 0;
		item->clip_position[1][0] = (float)item->clip->width;
		item->clip_position[1][1] = (float)item->clip->height;
	}
	else
	{
		item->clip_position[0][0] = clip_position[0][0];
		item->clip_position[0][1] = clip_position[0][1];
		item->clip_position[1][0] = clip_position[1][0];
		item->clip_position[1][1] = clip_position[1][1];
	}
	item->zoom = zoom;
	item->clip_zoom = clip_zoom;
	item->rotate = rotate;
	item->clip_rotate = clip_rotate;
	item->color = color;
	item->program = &programs->clip_draw;

	item->base_data.draw = (void(*)(DRAW_ITEM_BASE*))ClipDraw;
	item->base_data.delete_func = (void(*)(DRAW_ITEM_BASE*))DeleteClipDrawItem;
}

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
DRAW_ITEM_BASE* ClipDrawItemNew(
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
)
{
	CLIP_DRAW_ITEM *item = (CLIP_DRAW_ITEM*)MEM_ALLOC_FUNC(sizeof(*item));

	InitializeClipDrawItem(item, texture, clip, x, y, texture_position, clip_position,
		zoom, clip_zoom, rotate, clip_rotate, color, programs);

	return (DRAW_ITEM_BASE*)item;
}

#ifdef __cplusplus
}
#endif