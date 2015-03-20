#ifndef _INCLUDED_TEXT_H_
#define _INCLUDED_TEXT_H_

#include <ft2build.h>
#include FT_FREETYPE_H
#include "types.h"

/*
 eTEXT_IMAGE_SOURCE_TYPE�񋓑�
 �t�H���g�f�[�^�̃\�[�X�^�C�v
*/
typedef enum _eTEXT_IMAGE_SOURCE_TYPE
{
	TEXT_IMAGE_SOURCE_TYPE_MEMORY,	// ��������
	TEXT_IMAGE_SOURCE_TYPE_FILE		// �t�@�C��
} eTEXT_IMAGE_SOURCE_TYPE;
typedef struct _TEXT_IMAGE
{
	eTEXT_IMAGE_SOURCE_TYPE source_type;	// �t�H���g�f�[�^�̃^�C�v(��������܂��̓t�@�C��)
	FT_Library library;						// FreeType���C�u�������Ǘ�����f�[�^
	FT_Face face;							// �t�H���g�̃C���[�W
	union
	{
		uint8 *buffer;		// ��������̃t�H���g�f�[�^
		char *file_path;	// �t�H���g�t�@�C���̃p�X
	} font_data;
} TEXT_IMAGE;

/*
 TEXT_DRAW�\����
 �e�L�X�g�`��p�̃f�[�^���Ǘ�
*/
typedef struct _TEXT_DRAW
{
	TEXT_IMAGE text_image;	// FreeType�֘A�̃f�[�^
	int width;				// �����T�C�Y�̕�
	int height;				// �����T�C�Y�̍���
} TEXT_DRAW;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeTextDrawFromFile�֐�
 �e�L�X�g�`��p�̃f�[�^���t�H���g�t�@�C�����g���ď�����
 ����
 text_draw	: �e�L�X�g�`��p�̃f�[�^
 file_path	: �t�H���g�t�@�C���̃p�X
 �Ԃ�l
	����:TRUE	���s:FALSE
*/
EXTERN int InitializeTextDrawFromFile(
	TEXT_DRAW* text_draw,
	const char* file_path
);

/*
 InitializeTextDrawFromMemory�֐�
 �e�L�X�g�`��p�̃f�[�^����������̃t�H���g�f�[�^���g���ď�����
 ����
 text_draw		: �e�L�X�g�`��p�̃f�[�^
 font_data		: ��������̃t�H���g�f�[�^
 font_data_size	: �t�H���g�f�[�^�̃o�C�g��
 �Ԃ�l
	����:TRUE	���s:FALSE
*/
EXTERN int InitializeTextDrawFromMemory(
	TEXT_DRAW* text_draw,
	uint8* font_data,
	size_t font_data_size
);

/*
 ReleaseTextDraw�֐�
 �e�L�X�g�`��p�̃f�[�^���J������
 ����
 text_draw	: �e�L�X�g�`��p�̃f�[�^
*/
EXTERN void ReleaseTextDraw(TEXT_DRAW* text_draw);

/*
 TextDrawSetCharacterSize�֐�
 �����T�C�Y��ݒ肷��
 ����
 text_draw	: �e�L�X�g�`��p�̃f�[�^
 width		: �����T�C�Y(��)
 height		: �����T�C�Y(����)
 �Ԃ�l
	����:TRUE	���s:FALSE
*/
EXTERN int TextDrawSetCharacterSize(
	TEXT_DRAW* text_draw,
	int width,
	int height
);

/*
 TextDrawRender�֐�
 ������̕`������s����
 ����
 text_draw		: �e�L�X�g�̕`����Ǘ�����f�[�^
 utf8_text		: �`�悷�镶����(UTF-8)
 num_character	: �`�悷�镶����
 width			: �`�挋�ʂ̕����󂯂�ϐ��̃A�h���X
 height			: �`�挋�ʂ̍������󂯂�ϐ��̃A�h���X
 �Ԃ�l
	����:�s�N�Z���f�[�^		���s:NULL
*/
EXTERN uint8* TextDrawRender(
	TEXT_DRAW* text_draw,
	const char* utf8_text,
	int num_character,
	int* width,
	int* height
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TEXT_H_
