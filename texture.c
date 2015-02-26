#include <string.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "texture.h"
#include "image_file.h"
#include "utils.h"
#include "types.h"

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
GLuint GenerateTexture(
	uint8* pixels,
	int width,
	int height,
	int channel
)
{
	GLuint texture_id;		// �������ꂽ�e�N�X�`����ID

	// OpenGL�Ƀe�N�X�`���������˗�����ID��Ⴄ
	glGenTextures(1, &texture_id);

	// ID��0�Ȃ琶�����s
	if(texture_id == 0)
	{
		return 0;
	}

	// ���������e�N�X�`�����Ăяo��
		// (�e�N�X�`���̐ݒ�ƃs�N�Z���f�[�^�̈����n�����s������
	glBindTexture(GL_TEXTURE_2D, texture_id);

	// �e�N�X�`�����g��E�k�������Ƃ��̐ݒ�(���`���)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// �摜�̃`�����l�����ŏ�����؂�ւ�
	switch(channel)
	{
	case 1:	// �O���[�X�P�[���̉摜�̓��`�����l���݂̂̃e�N�X�`���ɂ���
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA,
			GL_UNSIGNED_BYTE, pixels);
		break;
	case 2:	// �O���[�X�P�[��+���`�����l���̉摜��RGBA�ɕϊ�����
		{
			uint8 *texture_pixels = (uint8*)MEM_ALLOC_FUNC(width*height*4);
			int i;
			for(i=0; i<width*height; i++)
			{
				texture_pixels[i*4] = pixels[i*2];
				texture_pixels[i*4+1] = pixels[i*2];
				texture_pixels[i*4+2] = pixels[i*2];
				texture_pixels[i*4+3] = pixels[i*2+1];
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, texture_pixels);

			MEM_FREE_FUNC(texture_pixels);
		}
		break;
	case 3:	// RGB�̉摜�f�[�^�͂��̂܂܃e�N�X�`���ɂ���
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB,
			GL_UNSIGNED_BYTE, pixels);
		break;
	case 4:	// RGBA�̉摜�f�[�^�����̂܂܃e�N�X�`���ɂ���
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, pixels);
		break;
	default:	// ����ȊO�̃t�H�[�}�b�g�͖��Ή�
		glDeleteTextures(1, &texture_id);
		return 0;
	}

	// �e�N�X�`���̌Ăяo�����I��
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture_id;
}

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
void InitializeTexture2D(
	TEXTURE_BASE* texture,
	uint8* pixels,
	int width,
	int height,
	int channel
)
{
	(void)memset(texture, 0, sizeof(*texture));

	texture->id = GenerateTexture(pixels, width, height, channel);
	texture->type = GL_TEXTURE_2D;
	texture->width = width;
	texture->height = height;
	texture->channel = channel;
}

/*
 ReleaseTexture2D�֐�
 2D�e�N�X�`�����J������
 ����
 texture	: �e�N�X�`���̊�{�f�[�^�\����
*/
void ReleaseTexture2D(TEXTURE_BASE* texture)
{
	glDeleteTextures(1, &texture->id);
	texture->id = 0;
}

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
int InitializeImageTexture(
	TEXTURE_BASE* texture,
	const char* path,
	void* (*open_func)(const char*, const char*, void*),
	size_t(*read_func)(void*, size_t, size_t, void*),
	int(*seek_func)(void*, long, int),
	long(*tell_func)(void*),
	int(*close_func)(void*, void*),
	void* user_data
)
{
	const char *extention;	// �g���q
	const char *str;		// ������̎Q�ƈʒu
	void *stream;			// �摜�ǂݍ��ݗp�X�g���[��
	uint8 *pixels = NULL;	// �摜�̃s�N�Z���f�[�^
	int width;				// �摜�̕�
	int height;				// �摜�̍���
	int channel;			// �摜�̃`�����l����

	// �t�@�C�����J��
	stream = open_func(path, "rb", user_data);
	if(stream == NULL)
	{	// �t�@�C���I�[�v�����s
		return FALSE;
	}

	// �g���q���擾
	extention = str = path;
	while(*str != '\0')
	{
		if(*str == '.')
		{
			extention = str;
		}

		str++;
	}

	// �摜�f�[�^���f�R�[�h����
	if(StringCompareIgnoreCase(extention, ".png") == 0)
	{
		pixels = ReadPngStream(stream, read_func,
			&width, &height, &channel);
	}
	else if(StringCompareIgnoreCase(extention, ".jpg") == 0
		|| StringCompareIgnoreCase(extention, ".jpeg") == 0)
	{
		long data_size;
		(void)seek_func(stream, 0, SEEK_END);
		data_size = tell_func(stream);
		(void)seek_func(stream, 0, SEEK_SET);
		pixels = ReadJpegStream(stream, read_func, data_size,
			&width, &height, &channel);
	}

	if(pixels == NULL)
	{
		(void)close_func(stream, user_data);
		return FALSE;
	}

	// OpenGL�̃e�N�X�`���𐶐�
	InitializeTexture2D(texture, pixels, width, height, channel);

	// ���������J��
	MEM_FREE_FUNC(pixels);
	(void)close_func(stream, user_data);

	return texture->id != 0;
}

#ifdef __cplusplus
}
#endif
