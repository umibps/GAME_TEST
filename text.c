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
 InitializeTextDrawFromFile関数
 テキスト描画用のデータをフォントファイルを使って初期化
 引数
 text_draw	: テキスト描画用のデータ
 file_path	: フォントファイルのパス
 返り値
	成功:TRUE	失敗:FALSE
*/
int InitializeTextDrawFromFile(
	TEXT_DRAW* text_draw,
	const char* file_path
)
{
	FT_Error error_code;

	(void)memset(text_draw, 0, sizeof(*text_draw));

	// FreeTypeの初期化
	FT_Init_FreeType(&text_draw->text_image.library);
	// フォントのデータをロード
	if((error_code = FT_New_Face(text_draw->text_image.library,
		file_path, 0, &text_draw->text_image.face)) != 0)
	{
		return FALSE;
	}

	text_draw->text_image.font_data.file_path = MEM_STRDUP_FUNC(file_path);

	return TRUE;
}

/*
 InitializeTextDrawFromMemory関数
 テキスト描画用のデータをメモリ上のフォントデータを使って初期化
 引数
 text_draw		: テキスト描画用のデータ
 font_data		: メモリ上のフォントデータ
 font_data_size	: フォントデータのバイト数
 返り値
	成功:TRUE	失敗:FALSE
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

	// FreeTypeの初期化
	FT_Init_FreeType(&text_draw->text_image.library);
	// フォントのデータをロード
	if((error_code = FT_New_Memory_Face(text_draw->text_image.library,
		font_data, (FT_Long)font_data_size, 0, &text_draw->text_image.face)) != 0)
	{
		return FALSE;
	}

	return TRUE;
}

/*
 ReleaseTextDraw関数
 テキスト描画用のデータを開放する
 引数
 text_draw	: テキスト描画用のデータ
*/
void ReleaseTextDraw(TEXT_DRAW* text_draw)
{
	FT_Done_Face(text_draw->text_image.face);
	FT_Done_FreeType(text_draw->text_image.library);

	MEM_FREE_FUNC(text_draw->text_image.font_data.buffer);
}

/*
 TextDrawSetCharacterSize関数
 文字サイズを設定する
 引数
 text_draw	: テキスト描画用のデータ
 width		: 文字サイズ(幅)
 height		: 文字サイズ(高さ)
 返り値
	成功:TRUE	失敗:FALSE
*/
int TextDrawSetCharacterSize(
	TEXT_DRAW* text_draw,
	int width,
	int height
)
{
	text_draw->width = width;
	text_draw->height = height;

	// FreeTypeのサイズを設定
	if(FT_Set_Pixel_Sizes(text_draw->text_image.face,
		width, height) != 0)
	{
		return FALSE;
	}

	return TRUE;
}

/*
 GetTextSize関数
 文字列描画サイズを取得する
 引数
 text				: 描画する文字列(UTF-8)
 num_character		: 描画する文字数
 character_width	: 1文字の幅
 character_height	: 1文字の高さ
 width				: 文字列描画の幅を受ける変数のアドレス
 height				: 文字列描画の高さを受ける変数のアドレス
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

	// 1文字ずつ処理
	for(i=0; i<num_character && *str != '\0'; i++)
	{
		if(*str == '\n')	// 改行なら
		{	// 最大幅のチェックをして高さ更新
			if(max_width < row_width)
			{
				max_width = row_width;
			}
			row_width = 0;
			local_height += character_height;
		}
		else
		{	// 改行以外なら文字の幅を更新
			row_width += character_width;
		}

		str = GetNextUtf8Character(str);
	}
	if(max_width < row_width)
	{
		max_width = row_width;
	}
	local_height += character_height;

	// 値を設定して終了
	*width = max_width;
	*height = local_height;
}

/*
 TextDrawRender関数
 文字列の描画を実行する
 引数
 text_draw		: テキストの描画を管理するデータ
 utf8_text		: 描画する文字列(UTF-8)
 num_character	: 描画する文字数
 width			: 描画結果の幅を受ける変数のアドレス
 height			: 描画結果の高さを受ける変数のアドレス
 返り値
	成功:ピクセルデータ		失敗:NULL
*/
uint8* TextDrawRender(
	TEXT_DRAW* text_draw,
	const char* utf8_text,
	int num_character,
	int* width,
	int* height
)
{
	uint8 *pixels;					// ピクセルデータ
	uint8 *work_pixels;				// 作業用ピクセルデータ
	const char *str = utf8_text;	// 描画する文字
	const char *next;				// 次に描画する文字
	const char *p;					// 描画文字の文字コード作成用
	FT_ULong code;					// 描画文字の文字コード
	FT_GlyphSlot slot;				// 描画結果へのアクセス用
	int local_width, local_height;	// 描画した文字列のサイズ
	int max_width;					// 　　　　〃
	int stride;						// 作業用ピクセルデータの幅
	int add_height_of_last_line;	// 「g」等のベースラインより下にも描画する文字が
										// 最後の行に来たら、高さを追加する
	int add_height;					// 上記の最大値検索用
	uint8 *dst;						// ピクセルデータのコピー先
	int i, j;

	// 描画する文字数にマイナスの値が指定されたら
		// 文字列全体を描画するように設定
	if(num_character < 0)
	{
		num_character = (int)strlen(utf8_text);
	}

	// 描画する
	GetTextSize(utf8_text, num_character, text_draw->width, text_draw->height,
		&local_width, &local_height);
	stride = local_width;

	// 作業用ピクセルデータを確保
	work_pixels = (uint8*)MEM_ALLOC_FUNC(local_width * local_height);
	(void)memset(work_pixels, 0, local_width * local_height);

	// 一文字ずつ描画する
	local_width = local_height = max_width = add_height_of_last_line = 0;
	slot = text_draw->text_image.face->glyph;
	for(i=0; i<num_character && *str != '\0'; i++)
	{
		// 次の文字を取得しておく
		next = GetNextUtf8Character(str);
		
		// 改行なら位置更新
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

		// Byte Order Markなら次の文字へ
		if((((uint32)(*str) & 0xC0) == 0x80)
			|| *str == 0xFE || *str == 0xFF)
		{
			str++;
			i--;
			continue;
		}

		// UTF-8→UTF-32しておく
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

		// 描画実施
		if(FT_Load_Char(text_draw->text_image.face, code, FT_LOAD_RENDER) != 0)
		{	// 失敗したら何もせずに次の文字へ
			str = next;
			continue;
		}

		// 描画結果をコピーする
		dst = &work_pixels[(local_height+text_draw->height-slot->bitmap_top)*stride + local_width + slot->bitmap_left];
		for(j=0; j<(int)slot->bitmap.rows; j++)
		{
			(void)memcpy(&dst[j*stride], &slot->bitmap.buffer[j*slot->bitmap.pitch], slot->bitmap.pitch);
		}

		// 「g」等のベースラインより低い部分にも描画する場合は最終の高さを調整
		add_height = slot->bitmap.rows - slot->bitmap_top;
		if(add_height > add_height_of_last_line)
		{
			add_height_of_last_line = add_height;
		}

		// 描画サイズを更新
		local_width += slot->advance.x >> 6;

		str = next;
	}
	local_height += text_draw->height + add_height_of_last_line;
	if(max_width < local_width)
	{
		max_width = local_width;
	}

	// 作業用ピクセルデータから最終データへコピー
	pixels = (uint8*)MEM_ALLOC_FUNC(max_width * local_height);
	(void)memset(pixels, 0, max_width * local_height);
	for(i=0; i<local_height; i++)
	{
		(void)memcpy(&pixels[i*max_width], &work_pixels[i*stride], max_width);
	}

	// 値の返却
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
