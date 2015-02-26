#ifndef _INCLUDED_VERTEX_H_
#define _INCLUDED_VERTEX_H_

#include <GL/glew.h>
#include <GL/glut.h>
#include "types.h"

/*
 VERTEX_BUFFER�\����
 �`�掞��OpenGL�̃o�b�t�@���Ǘ�
*/
typedef struct _VERTEX_BUFFER
{
	GLuint vertex_array;	// �f�[�^�z�u���L������o�b�t�@
	GLuint vertex_buffer;	// ���_�o�b�t�@�I�u�W�F�N�g
	GLuint index_buffer;	// �C���f�b�N�X�o�b�t�@�I�u�W�F�N�g
} VERTEX_BUFFER;

/*
 DRAW_VERTEX�\����
 �`�掞�̒��_�f�[�^�̍\��
*/
typedef struct _DRAW_VERTEX
{
	float position[3];		// �`�悷����W(XYZ)
	float texture_coord[2];	// �\��t����e�N�X�`���̍��W(UV)
	uint8 color[4];			// �ݒ肷��F(RGBA)
} DRAW_VERTEX;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeVertexBuffer�֐�
 ���_�o�b�t�@������������
 ����
 buffer			: ���_�o�b�t�@���Ǘ�����\����
 vertex_size	: ���_�o�b�t�@�̃T�C�Y
 index_size		: �C���f�b�N�X�o�b�t�@�̃T�C�Y
 data_stride	: ���_�f�[�^�\���̂̃o�C�g��
 vertex_key		: GLSL�ɓn�������_���W�̎���ID
 vertex_offset	: ���_���W�f�[�^�̈ʒu
 texture_key	: GLSL�ɓn�����e�N�X�`�����W�̎���ID
 texture_offset	: �e�N�X�`�����W�f�[�^�̈ʒu
 color_key		: GLSL�ɓn�����F�f�[�^�̎���ID
 color_offset	: �F�f�[�^�̈ʒu
*/
EXTERN void InitializeVertexBuffer(
	VERTEX_BUFFER* buffer,
	size_t vertex_size,
	size_t index_size,
	size_t data_stride,
	int vertex_key,
	void* vertex_offset,
	int texture_key,
	void* texture_offset,
	int color_key,
	void* color_offset
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_VERTEX_H_
