#include <string.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "text.h"
#include "utils.h"

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
int InitializeTextDrawFromFile(
	TEXT_DRAW* text_draw,
	const char* file_path
)
{
	FT_Error error_code;

	(void)memset(text_draw, 0, sizeof(*text_draw));

	// FreeType�̏�����
	FT_Init_FreeType(&text_draw->text_image.library);
	// �t�H���g�̃f�[�^�����[�h
	if((error_code = FT_New_Face(text_draw->text_image.library,
		file_path, 0, &text_draw->text_image.face)) != 0)
	{
		return FALSE;
	}

	text_draw->text_image.font_data.file_path = MEM_STRDUP_FUNC(file_path);

	return TRUE;
}

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
int InitializeTextDrawFromMemory(
	TEXT_DRAW* text_draw,
	uint8* font_data,
	size_t font_data_size
)
{
	FT_Error error_code;

	(void)memset(text_draw, 0, sizeof(*text_draw));

	text_draw->text_image.font_data.buffer = (uint8*)MEM_ALLOC_FUNC(font_data_size);
	(void)memcpy(text_draw->text_image.font_data.buffer, font_data, font_data_size);

	// FreeType�̏�����
	FT_Init_FreeType(&text_draw->text_image.library);
	// �t�H���g�̃f�[�^�����[�h
	if((error_code = FT_New_Memory_Face(text_draw->text_image.library,
		font_data, (FT_Long)font_data_size, 0, &text_draw->text_image.face)) != 0)
	{
		return FALSE;
	}

	return TRUE;
}

/*
 ReleaseTextDraw�֐�
 �e�L�X�g�`��p�̃f�[�^���J������
 ����
 text_draw	: �e�L�X�g�`��p�̃f�[�^
*/
void ReleaseTextDraw(TEXT_DRAW* text_draw)
{
	FT_Done_Face(text_draw->text_image.face);
	FT_Done_FreeType(text_draw->text_image.library);

	MEM_FREE_FUNC(text_draw->text_image.font_data.buffer);
}

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
int TextDrawSetCharacterSize(
	TEXT_DRAW* text_draw,
	int width,
	int height
)
{
	text_draw->width = width;
	text_draw->height = height;

	// FreeType�̃T�C�Y��ݒ�
	if(FT_Set_Pixel_Sizes(text_draw->text_image.face,
		width, height) != 0)
	{
		return FALSE;
	}

	return TRUE;
}

/*
 GetTextSize�֐�
 ������`��T�C�Y���擾����
 ����
 text				: �`�悷�镶����(UTF-8)
 num_character		: �`�悷�镶����
 character_width	: 1�����̕�
 character_height	: 1�����̍���
 width				: ������`��̕����󂯂�ϐ��̃A�h���X
 height				: ������`��̍������󂯂�ϐ��̃A�h���X
*/
static void GetTextSize(
	const char* text,
	int num_character,
	int character_width,
	int character_height,
	int* width,
	int* height
)
{
	const char *str = text;
	int row_width = 0;
	int max_width = 0;
	int local_height = character_height;
	int i;

	// 1����������
	for(i=0; i<num_character && *str != '\0'; i++)
	{
		if(*str == '\n')	// ���s�Ȃ�
		{	// �ő啝�̃`�F�b�N�����č����X�V
			if(max_width < row_width)
			{
				max_width = row_width;
			}
			row_width = 0;
			local_height += character_height;
		}
		else
		{	// ���s�ȊO�Ȃ當���̕����X�V
			row_width += character_width;
		}

		str = GetNextUtf8Character(str);
	}
	if(max_width < row_width)
	{
		max_width = row_width;
	}
	local_height += character_height;

	// �l��ݒ肵�ďI��
	*width = max_width;
	*height = local_height;
}

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
uint8* TextDrawRender(
	TEXT_DRAW* text_draw,
	const char* utf8_text,
	int num_character,
	int* width,
	int* height
)
{
	uint8 *pixels;					// �s�N�Z���f�[�^
	uint8 *work_pixels;				// ��Ɨp�s�N�Z���f�[�^
	const char *str = utf8_text;	// �`�悷�镶��
	const char *next;				// ���ɕ`�悷�镶��
	const char *p;					// �`�敶���̕����R�[�h�쐬�p
	FT_ULong code;					// �`�敶���̕����R�[�h
	FT_GlyphSlot slot;				// �`�挋�ʂւ̃A�N�Z�X�p
	int local_width, local_height;	// �`�悵��������̃T�C�Y
	int max_width;					// �@�@�@�@�V
	int stride;						// ��Ɨp�s�N�Z���f�[�^�̕�
	int add_height_of_last_line;	// �ug�v���̃x�[�X���C����艺�ɂ��`�悷�镶����
										// �Ō�̍s�ɗ�����A������ǉ�����
	int add_height;					// ��L�̍ő�l�����p
	uint8 *dst;						// �s�N�Z���f�[�^�̃R�s�[��
	int i, j;

	// �`�悷�镶�����Ƀ}�C�i�X�̒l���w�肳�ꂽ��
		// ������S�̂�`�悷��悤�ɐݒ�
	if(num_character < 0)
	{
		num_character = (int)strlen(utf8_text);
	}

	// �`�悷��
	GetTextSize(utf8_text, num_character, text_draw->width, text_draw->height,
		&local_width, &local_height);
	stride = local_width;

	// ��Ɨp�s�N�Z���f�[�^���m��
	work_pixels = (uint8*)MEM_ALLOC_FUNC(local_width * local_height);
	(void)memset(work_pixels, 0, local_width * local_height);

	// �ꕶ�����`�悷��
	local_width = local_height = max_width = add_height_of_last_line = 0;
	slot = text_draw->text_image.face->glyph;
	for(i=0; i<num_character && *str != '\0'; i++)
	{
		// ���̕������擾���Ă���
		next = GetNextUtf8Character(str);
		
		// ���s�Ȃ�ʒu�X�V
		if(*str == '\n')
		{
			if(max_width < local_width)
			{
				max_width = local_width;
			}
			local_width = add_height_of_last_line = 0;
			local_height += text_draw->height;

			str++;
			continue;
		}

		// Byte Order Mark�Ȃ玟�̕�����
		if((((uint32)(*str) & 0xC0) == 0x80)
			|| *str == 0xFE || *str == 0xFF)
		{
			str++;
			i--;
			continue;
		}

		// UTF-8��UTF-32���Ă���
		switch((unsigned int)(next - str))
		{
		case 1:
			code = *str;
			break;
		case 2:
			code = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
			break;
		case 3:
			code = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
			break;
		case 4:
			code = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12)
				| ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
			break;
		case 5:
			code = ((str[0] & 0x03) << 24) | ((str[1] & 0x3F) << 18)
				| ((str[2] & 0x3F) << 12) | ((str[3] & 0x3F) << 6) | (str[4] & 0x3F);
			break;
		case 6:
			code = ((str[0] & 0x01) << 30) | ((str[1] & 0x3F) << 24)
				| ((str[2] & 0x3F) << 18) | ((str[3] & 0x3F) << 12)
					| ((str[4] & 0x3F) << 6) | (str[5] & 0x3F);
			break;
		default:
			MEM_FREE_FUNC(work_pixels);
			return NULL;
		}

		// �`����{
		if(FT_Load_Char(text_draw->text_image.face, code, FT_LOAD_RENDER) != 0)
		{	// ���s�����牽�������Ɏ��̕�����
			str = next;
			continue;
		}

		// �`�挋�ʂ��R�s�[����
		dst = &work_pixels[(local_height+text_draw->height-slot->bitmap_top)*stride + local_width + slot->bitmap_left];
		for(j=0; j<(int)slot->bitmap.rows; j++)
		{
			(void)memcpy(&dst[j*stride], &slot->bitmap.buffer[j*slot->bitmap.pitch], slot->bitmap.pitch);
		}

		// �ug�v���̃x�[�X���C�����Ⴂ�����ɂ��`�悷��ꍇ�͍ŏI�̍����𒲐�
		add_height = slot->bitmap.rows - slot->bitmap_top;
		if(add_height > add_height_of_last_line)
		{
			add_height_of_last_line = add_height;
		}

		// �`��T�C�Y���X�V
		local_width += slot->advance.x >> 6;

		str = next;
	}
	local_height += text_draw->height + add_height_of_last_line;
	if(max_width < local_width)
	{
		max_width = local_width;
	}

	// ��Ɨp�s�N�Z���f�[�^����ŏI�f�[�^�փR�s�[
	pixels = (uint8*)MEM_ALLOC_FUNC(max_width * local_height);
	(void)memset(pixels, 0, max_width * local_height);
	for(i=0; i<local_height; i++)
	{
		(void)memcpy(&pixels[i*max_width], &work_pixels[i*stride], max_width);
	}

	// �l�̕ԋp
	if(width != NULL)
	{
		*width = max_width;
	}
	if(height != NULL)
	{
		*height = local_height;
	}

	MEM_FREE_FUNC(work_pixels);

	return pixels;
}

#ifdef __cplusplus
}
#endif
