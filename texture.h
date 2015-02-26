#ifndef _INCLUDED_TEXTURE_H_
#define _INCLUDED_TEXTURE_H_

#include <GL/glew.h>
#include <GL/glut.h>
#include "types.h"

/*
 TEXTURE_BASE�\����
 �e�N�X�`���̊�{�f�[�^
*/
typedef struct _TEXTURE_BASE
{
	GLuint id;		// �e�N�X�`����ID
	GLenum type;	// �e�N�X�`���̃^�C�v(1D or 2D or 3D)
	int width;		// �e�N�X�`���摜�̕�
	int height;		// �e�N�X�`���摜�̍���
	int channel;	// �e�N�X�`���摜�̃`�����l����
} TEXTURE_BASE;

#ifdef __cplusplus
extern "C" {
#endif

/*
 GenerateTexgture�֐�
 �摜�̃s�N�Z���f�[�^��n��OpenGL�̃e�N�X�`���𐶐�����
 ����
 pixels		: �摜�̃s�N�Z���f�[�^
 width		: �摜�̕�
 height		: �摜�̍���
 channel	: �摜�̃`�����l����
				 (�O���[�X�P�[��:1�A�O���[�X�P�[��+���`�����l��:2�ARGB:3�ARGBA4)
 �Ԃ�l
 �������ꂽ�e�N�X�`����ID
*/
EXTERN GLuint GenerateTexture(
	uint8* pixels,
	int width,
	int height,
	int channel
);

/*
 InitializeTexture2D�֐�
 2D�e�N�X�`���𐶐�������������
 ����
 texture	: �e�N�X�`���̊�{�f�[�^�\����
 pixels		: �e�N�X�`���ɂ���摜�̃s�N�Z���f�[�^
 width		: �摜�̕�
 height		: �摜�̍���
 channel	: �摜�̃`�����l����
				(�O���[�X�P�[��:1�A�O���[�X�P�[��+���`�����l��:2�ARGB:3�ARGBA4)
*/
EXTERN void InitializeTexture2D(
	TEXTURE_BASE* texture,
	uint8* pixels,
	int width,
	int height,
	int channel
);

/*
 InitializeImageTexture�֐�
 �摜�t�@�C�����J���A�e�N�X�`���𐶐�����
 ����
 texture	: �e�N�X�`���̊�{�f�[�^�\����
 path		: �t�@�C���p�X
 open_func	: �t�@�C�����J���̂Ɏg���֐��|�C���^
 read_fucn	: �t�@�C���̓ǂݍ��݂Ɏg���֐��|�C���^
 seek_func	: �t�@�C���̃V�[�N�Ɏg���֐��|�C���^
 tell_func	: �t�@�C���̃V�[�N�ʒu�擾�Ɏg���֐��|�C���^
 close_func	: �t�@�C�������̂Ɏg���֐��|�C���^
 user_data	: �t�@�C�����쎞�Ɏg���������[�U�[��`�f�[�^
 �Ԃ�l
	����:TRUE	���s:FALSE
*/
EXTERN int InitializeImageTexture(
	TEXTURE_BASE* texture,
	const char* path,
	void* (*open_func)(const char*, const char*, void*),
	size_t(*read_func)(void*, size_t, size_t, void*),
	int(*seek_func)(void*, long, int),
	long(*tell_func)(void*),
	int(*close_func)(void*, void*),
	void* user_data
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TEXTURE_H_
