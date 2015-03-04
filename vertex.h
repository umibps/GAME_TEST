#ifndef _INCLUDED_VERTEX_H_
#define _INCLUDED_VERTEX_H_

#include <stdarg.h>
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
 �ϒ�����		: �z��̒���(int), �f�[�^��ID(int), �f�[�^�̌^(GLenum),
					�f�[�^�̐��K���L��(GL_TRUE or GL_FALSE), �f�[�^�̃I�t�Z�b�g(size_t)
					�Ŏw�肷�� (�Ō��0)
*/
EXTERN void InitializeVertexBuffer(
	VERTEX_BUFFER* buffer,
	size_t vertex_size,
	size_t index_size,
	size_t data_stride,
	int first_size,
	...
);

/*
 InitializeVertexArray�֐�
 ���_�o�b�t�@���g�p���Ȃ��ꍇ�̃f�[�^�z�u��ݒ肷��
 ����
 vertex_array	: �f�[�^�z�u���L�����钸�_�z��I�u�W�F�N�g
 vertex_size	: ���_�o�b�t�@�̃T�C�Y
 index_size		: �C���f�b�N�X�o�b�t�@�̃T�C�Y
 data_stride	: ���_�f�[�^�\���̂̃o�C�g��
 �ϒ�����		: �z��̒���(int), �f�[�^��ID(int), �f�[�^�̌^(GLenum),
					�f�[�^�̐��K���L��(GL_TRUE or GL_FALSE), �f�[�^�̃I�t�Z�b�g(size_t)
					�Ŏw�肷�� (�Ō��0)
*/
EXTERN void InitializeVertexArray(
	GLuint* vertex_array,
	size_t data_stride,
	int first_size,
	...
);

/*
 SetAttributePointer�֐�
 OpenGL�ɓn�����_�f�[�^�̔z�u��ݒ肷��
 ����
 stride		: ���_�f�[�^1���̃o�C�g��
 �ϒ�����	: �z��̒���(int), �f�[�^��ID(int), �f�[�^�̌^(GLenum),
				�f�[�^�̐��K���L��(GL_TRUE or GL_FALSE), �f�[�^�̃I�t�Z�b�g(size_t)
				�Ŏw�肷�� (�Ō��0)
*/
EXTERN void SetAttributePointer(
	size_t stride,
	int first_size,
	...
);

/*
 VaListAttributePointer�֐�
 va_list����OpenGL�ɓn���f�[�^�̔z�u��ݒ肷��
 ����
 stride		: ���_�f�[�^1���̃o�C�g��
 first_size	: �ŏ��̃f�[�^�̔z��
 list		: va_list(���e��SetAttributePointer�֐����Q��)
*/
EXTERN void VaListAttributePointer(va_list list, int first_size, size_t stride);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_VERTEX_H_
