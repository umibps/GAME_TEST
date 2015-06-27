#ifndef _INCLUDED_TEXTURE_H_
#define _INCLUDED_TEXTURE_H_

#include <GL/glew.h>
#include <GL/glut.h>
#include "types.h"
#include "text.h"
#include "utils.h"

/*
 TEXTURE_BASE�\����
 �e�N�X�`���̊�{�f�[�^
*/
typedef struct _TEXTURE_BASE
{
	GLuint id;				// �e�N�X�`����ID
	char *file_path;		// �摜�t�@�C���̃p�X
	GLenum type;			// �e�N�X�`���̃^�C�v(1D or 2D or 3D)
	int width;				// �e�N�X�`���摜�̕�
	int height;				// �e�N�X�`���摜�̍���
	int channel;			// �e�N�X�`���摜�̃`�����l����
	int reference_count;	// �Q�ƃJ�E���g
} TEXTURE_BASE;

/*
 TEXTURES�\����
 �v���O�����S�̂ŕێ����Ă���e�N�X�`�����Ǘ�
*/
typedef struct _IMAGE_TEXTURES
{
	STRING_HASH_TABLE hash_table;	// �n�b�V���e�[�u���Ń_�u���h�~
} IMAGE_TEXTURES;

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
 SetTextureWrap�֐�
 �e�N�X�`���̂͂ݏo�����̈�����ݒ肷��
 ����
 texture_id	: �e�N�X�`���I�u�W�F�N�g��ID
 wrap_mode	: �J��Ԃ�:GL_REPEAT, �N���b�s���O:GL_CLAMP
*/
EXTERN void SetTextureWrap(GLuint texture_id, GLenum wrap_mode);

/*
 TextureCountUpReference�֐�
 �e�N�X�`���̎Q�ƃJ�E���g�𑝂₷
 ����
 texture	: �Q�ƃJ�E���g�𑝂₷�e�N�X�`��
 �Ԃ�l
	�Q�ƃJ�E���g�𑝂₵���e�N�X�`��
*/
EXTERN TEXTURE_BASE* TextureCountUpReference(TEXTURE_BASE* texture);

/*
 InitializeTexture2D�֐�
 2D�e�N�X�`���𐶐�������������
 ����
 texture	: �e�N�X�`���̊�{�f�[�^�\����
 file_path	: �摜�t�@�C���̃p�X
 pixels		: �e�N�X�`���ɂ���摜�̃s�N�Z���f�[�^
 width		: �摜�̕�
 height		: �摜�̍���
 channel	: �摜�̃`�����l����
				(�O���[�X�P�[��:1�A�O���[�X�P�[��+���`�����l��:2�ARGB:3�ARGBA4)
*/
EXTERN void InitializeTexture2D(
	TEXTURE_BASE* texture,
	uint8* pixels,
	const char* file_path,
	int width,
	int height,
	int channel
);

/*
 DeleteTexture2D�֐�
 2D�e�N�X�`�����폜����
 ����
 texthre	: �폜����e�N�X�`��
*/
EXTERN void DeleteTexture2D(TEXTURE_BASE* texture);

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

/*
 InitializeTextTexture�֐�
 �e�L�X�g�̃e�N�X�`���𐶐�����
 ����
 texture		: �e�N�X�`���̊�{�f�[�^�\����
 text_draw		: �e�L�X�g�̕`����Ǘ�����f�[�^
 utf8_text		: �`�悷��e�L�X�g
 num_character	: �`�悷�镶����
 �Ԃ�l
	����:TRUE	���s:FALSE
*/
EXTERN int InitializeTextTexture(
	TEXTURE_BASE* texture,
	TEXT_DRAW* text_draw,
	const char* utf8_text,
	int num_character
);

/*
 TextTextureNew�֐�
 �e�L�X�g�̃e�N�X�`�������������m�ۂ��Đ�������
 ����
 text_draw		: �e�L�X�g�̕`����Ǘ�����f�[�^
 utf8_text		: �`�悷��e�L�X�g
 num_character	: �`�悷�镶����
 �Ԃ�l
	���������e�N�X�`���̃f�[�^(�s�v�ɂȂ�����MEM_FREE_FUNC)
*/
EXTERN TEXTURE_BASE* TextTextureNew(
	TEXT_DRAW* text_draw,
	const char* utf8_text,
	int num_character
);

/*
 ImageTextureNew�֐�
 �摜�t�@�C������e�N�X�`�����_�u��h�~���č쐬����
 ����
*/
EXTERN TEXTURE_BASE* ImageTextureNew(
	const char* path,
	void* (*open_func)(const char*, const char*, void*),
	size_t(*read_func)(void*, size_t, size_t, void*),
	int(*seek_func)(void*, long, int),
	long(*tell_func)(void*),
	int(*close_func)(void*, void*),
	void* user_data,
	IMAGE_TEXTURES* textures
);

/*
 InitializeSquareTexture�֐�
 �����`�̃e�N�X�`���𐶐�����
 ����
 texture	: �e�N�X�`���̊�{�f�[�^�\����
 width		: �����`�̕�
 height		: �����`�̍���
 �Ԃ�l
	����:TRUE	���s:FALSE
*/
EXTERN int InitializeSquareTexture(
	TEXTURE_BASE* texture,
	int width,
	int height
);

/*
 SquareTextureNew�֐�
 �����`�̃e�N�X�`�������������m�ۂ��Đ�������
 ����
 width	: �����`�̕�
 height	: �����`�̍���
 �Ԃ�l
	���������e�N�X�`���̃f�[�^(�s�v�ɂȂ�����MEM_FREE_FUNC)
*/
EXTERN TEXTURE_BASE* SquareTextureNew(int width, int height);

/*
 InitializeImageTextures�֐�
 �e�N�X�`���S�̂��Ǘ�����f�[�^��������
 ����
 textures	: �e�N�X�`���S�̂��Ǘ�����f�[�^
*/
EXTERN void InitializeImageTextures(IMAGE_TEXTURES* textures);

/*
 ReleaseImageTextures�֐�
 �e�N�X�`���S�̂��Ǘ�����f�[�^���J��
 ����
 textures	: �e�N�X�`���S�̂��Ǘ�����f�[�^
*/
EXTERN void ReleaseImageTextures(IMAGE_TEXTURES* textures);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TEXTURE_H_
