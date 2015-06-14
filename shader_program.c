#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include "shader_program.h"
#include "configure.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 AddShaderSource関数
 GLSLのプログラムオブジェクトにシェーダーオブジェクトを追加
 引数
 program_id	: プログラムオブジェクト
 source		: GLSLのシェーダーソースコード
 type		: シェーダーのタイプ(頂点シェーダー or フラグメントシェーダー)
 返り値
	成功:TRUE	失敗:FALSE
*/
static int AddShaderSource(GLuint program_id, const char* source, GLenum type)
{
	GLint compiled;		// ソースコードのコンパイルの成功/失敗
	GLuint shader;		// シェーダーオブジェクト
	
	// シェーダーオブジェクトを生成
	shader = glCreateShader(type);

	// 生成したシェーダーオブジェクトにソースコードを渡す
	glShaderSource(shader, 1, &source, NULL);
	// 渡したソースコードをコンパイル
	glCompileShader(shader);
	// コンパイルに成功したか確認
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	// compiledの中身が0ならコンパイル失敗
	if(compiled == 0)
	{
		GLint length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		if(length > 0)
		{
			char *message = (char*)malloc(length);
			glGetShaderInfoLog(shader, length, &length, message);
			(void)printf("%s\n", message);
			free(message);
		}
		glDeleteShader(shader);
		return FALSE;
	}

	// 
	glAttachShader(program_id, shader);
	glDeleteShader(shader);

	return TRUE;
}

/*
 GenerateGlslProgram関数
 GLSLのプログラムオブジェクトを生成し、シェーダーをコンパイルする
 引数
 vertex_shader_source	: 頂点シェーダーのソースコード文字列
 fragment_shader_source	: フラグメントシェーダーのソースコード文字列
 返り値
	生成したプログラムオブジェクトのID
*/
GLuint GenerateGlslProgram(
	const char* vertex_shader_source,
	const char* fragment_shader_source
)
{
	GLuint program_id;	// プログラムオブジェクトのID

	// プログラムオブジェクトの生成をOpenGLに依頼
	program_id = glCreateProgram();

	// 頂点シェーダーを追加
	if(AddShaderSource(program_id, vertex_shader_source, GL_VERTEX_SHADER) == FALSE)
	{
		glDeleteProgram(program_id);
		return 0;
	}

	// フラグメントシェーダーを追加
	if(fragment_shader_source != NULL)
	{
		if(AddShaderSource(program_id, fragment_shader_source, GL_FRAGMENT_SHADER) == FALSE)
		{
			glDeleteProgram(program_id);
			return 0;
		}
	}

	return program_id;
}

/*
 ReleaseShaderProgram関数
 シェーダープログラムを開放する
 引数
 program	: シェーダープログラムの基本データ
*/
void ReleaseShaderProgram(SHADER_PROGRAM_BASE* program)
{
	glDeleteProgram(program->program_id);
	program->program_id = 0;
}

/*
 ShaderProgramLink関数
 シェーダープログラムをリンクする
 引数
 program	: シェーダープログラムの基本データ
 返り値
	成功:TRUE	失敗:FALSE
*/
int ShaderProgramLink(SHADER_PROGRAM_BASE* program)
{
	GLint status;	// リンクの成功/失敗を受ける

	// リンク実行
	glLinkProgram(program->program_id);
	// リンクに成功したか調べる
	glGetProgramiv(program->program_id, GL_LINK_STATUS, &status);
	if(status == 0)
	{
		GLint length;
		glGetProgramiv(program->program_id, GL_INFO_LOG_LENGTH, &length);
		if(length > 0)
		{
			char *message = (char*)malloc(length);
			glGetProgramInfoLog(program->program_id, length, &length, message);
			(void)printf("%s\n", message);
			free(message);
		}
		ReleaseShaderProgram(program);
		return FALSE;
	}

	return TRUE;
}

/*
 InitializeShaderProgram関数
 1つ分のGLSLのプログラマブルシェーダー管理データを初期化
 引数
 program				: シェーダープログラムの基本データ
 vertex_shader_source	: 頂点シェーダーのソースコード文字列
 fragment_shader_source	: フラグメントシェーダーのソースコード文字列
 可変長引数				: 頂点シェーダー中のin修飾の変数名とそれに割り当てるID
							(const char*の名前, GLuintのkeyを交互に指定. 最後にNULLを指定)
 返り値
	成功:TRUE	失敗:FALSE
*/
int InitializeShaderProgram(
	SHADER_PROGRAM_BASE* program,
	const char* vertex_shader_source,
	const char* fragment_shader_source,
	const char* first_attribute,
	...
)
{
	const char *attribute_name;	// 入力変数名
	va_list list;				// 可変長引数のリスト

	(void)memset(program, 0, sizeof(*program));

	if((program->program_id = GenerateGlslProgram(
		vertex_shader_source, fragment_shader_source)) == 0)
	{
		return FALSE;
	}

	// 入力変数の名前とIDを結び付ける
	attribute_name = first_attribute;
	va_start(list, first_attribute);
	while(attribute_name != NULL)
	{
		GLuint key = va_arg(list, GLuint);
		glBindAttribLocation(program->program_id, key, attribute_name);
		attribute_name = va_arg(list, const char*);
	}
	va_end(list);

	// コンパイルしたシェーダーをリンク
	if(ShaderProgramLink(program) == FALSE)
	{
		ReleaseShaderProgram(program);
		return FALSE;
	}

	return TRUE;
}

/*
 DrawSquareProgramSetUniformLocation関数
 シェーダーオブジェクトからuniform属性の変数のIDを取得する
 引数
 program	: シェーダーを管理するデータ
*/
void DrawSquareProgramSetUniformLocation(DRAW_SQUARE_PROGRAM* program)
{
	program->position_uniform_location = glGetUniformLocation(program->base_data.program_id, "position");
	program->texture_size_uniform_location = glGetUniformLocation(program->base_data.program_id, "texture_size");
	program->reverse_half_display_size_unifrom_location = glGetUniformLocation(program->base_data.program_id, "reverse_display_half_size");
	program->display_height_uniform_location = glGetUniformLocation(program->base_data.program_id, "display_height");
	program->zoom_uniform_location = glGetUniformLocation(program->base_data.program_id, "zoom");
	program->rotate_uniform_location = glGetUniformLocation(program->base_data.program_id, "rotate");

	program->color_uniform_location = glGetUniformLocation(program->base_data.program_id, "color");
	program->texture_uniform_location = glGetUniformLocation(program->base_data.program_id, "draw_texture");
}

/*
 InitializeDrawSquareProgram関数
 画像全体を使って描画するシェーダープログラムの初期化
 引数
 program		: シェーダーを管理するデータ
 display_width	: 描画領域の幅
 display_height	: 描画領域の高さ
 返り値
	成功:TRUE	失敗:FALSE
*/
int InitializeDrawSquareProgram(
	DRAW_SQUARE_PROGRAM* program,
	float display_width,
	float display_height
)
{
	// 頂点シェーダーのソースコード
	const char vertex_shader[] =
		"#if __VERSION__ < 130\n"
		"#define in attribute\n"
		"#define out varying\n"
		"#endif\n"
		"in vec3 in_position;\n"
		"in vec2 in_texture_coordinate;\n"
		"out vec2 out_texture_coordinate;\n"
		"uniform vec2 position;\n"
		"uniform vec2 texture_size;\n"
		"uniform vec2 display_half_size;\n"
		"uniform vec2 reverse_display_half_size;\n"
		"uniform float display_height;\n"
		"uniform float zoom;\n"
		"uniform float rotate;\n"
		"void main()\n"
		"{\n"
			"\tfloat x;\n"
			"\tfloat y;\n"
			"\tfloat rotate_x;\n"
			"\tfloat rotate_y;\n"
			"\tfloat sin_value;\n"
			"\tfloat cos_value;\n"
			"\tx = (- (texture_size.x * 0.5) + texture_size.x * in_position.x) * zoom;\n"
			"\ty = (- (texture_size.y * 0.5) + texture_size.y * in_position.y) * zoom;\n"
			"\tsin_value = sin(rotate);\n"
			"\tcos_value = cos(rotate);\n"
			"\trotate_x = x * cos_value + y * sin_value;\n"
			"\trotate_y = - x * sin_value + y * cos_value;\n"
			"\tgl_Position = vec4((rotate_x + texture_size.x * 0.5 + position.x) * reverse_display_half_size.x - 1,\n"
				"\t\t(display_height - rotate_y - texture_size.y * 0.5 - position.y) * reverse_display_half_size.y - 1, in_position.z, 1);\n"
			"\tout_texture_coordinate = in_texture_coordinate;\n"
		"}";
	const char fragment_shader[] =
		"#if __VERSION__ < 130\n"
		"#define in varying\n"
		"#define out_pixel_color gl_FragColor\n"
		"#define texture(samp, uv) texture2D((samp), (uv))\n"
		"#else\n"
		"out out_pixel_color;\n"
		"#endif\n"
		"in vec2 out_texture_coordinate;\n"
		"uniform vec4 color;\n"
		"uniform sampler2D draw_texture;\n"
		"void main()\n"
		"{\n"
			"\tout_pixel_color = color * texture2D(draw_texture, out_texture_coordinate);\n"
		"}\n";
	// 頂点バッファ
	const DRAW_SQUARE_VERTEX vertices[4] =
	{
		{{0, 0, 0}, {0, 0}},
		{{1, 0, 0}, {1, 0}},
		{{1, 1, 0}, {1, 1}},
		{{0, 1, 0}, {0, 1}}
	};
	// インデックス
	const unsigned char indices[4] = {0, 1, 2, 3};

	// シェーダープログラムを設定
	if(InitializeShaderProgram(&program->base_data, vertex_shader, fragment_shader,
			"in_position", SHADER_ATTRIBUTE_VERTEX,
			"in_texture_coordinate", SHADER_ATTRIBUTE_TEXTURE_COORD,
			NULL
		)
		== FALSE
	)
	{
		return FALSE;
	}

	// uniform属性の変数識別IDを取得
	DrawSquareProgramSetUniformLocation(program);

	// uniform変数の初期値を設定する
	glUseProgram(program->base_data.program_id);
	glUniform2f(program->reverse_half_display_size_unifrom_location,
		1 / (display_width * 0.5f), 1 / (display_height * 0.5f));
	glUniform1f(program->display_height_uniform_location, (GLfloat)display_height);
	glUniform1f(program->zoom_uniform_location, 1.0f);
	glUniform1f(program->rotate_uniform_location, 0.0f);
	glUniform4f(program->color_uniform_location, 1.0f, 1.0f, 1.0f, 1.0f);
	glUseProgram(0);

	// 頂点バッファを初期化
	InitializeVertexBuffer(
		&program->vertex_buffer,
		sizeof(vertices),
		(void*)vertices,
		sizeof(indices),
		(void*)indices,
		sizeof(*vertices),
		3, SHADER_ATTRIBUTE_VERTEX, GL_FLOAT, GL_FALSE, NULL,
		2, SHADER_ATTRIBUTE_TEXTURE_COORD, GL_FLOAT, GL_FALSE, offsetof(DRAW_SQUARE_VERTEX, texture_coordinate),
		0
	);

	// 頂点バッファにデータを渡す
	glBindBuffer(GL_ARRAY_BUFFER, program->vertex_buffer.vertex_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, program->vertex_buffer.index_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return TRUE;
}

/*
 ClipDrawProgramSetUniformLocation関数
 シェーダーオブジェクトからuniform属性の変数のIDを取得する
 引数
 program	: シェーダーを管理するデータ
*/
void ClipDrawProgramSetUniformLocation(CLIP_DRAW_PROGRAM* program)
{
	program->position_uniform_location = glGetUniformLocation(program->base_data.program_id, "position");
	program->texture_position_uniform_location = glGetUniformLocation(program->base_data.program_id, "texture_position");
	program->clip_position_uniform_location = glGetUniformLocation(program->base_data.program_id, "clip_position");
	program->texture_size_uniform_location = glGetUniformLocation(program->base_data.program_id, "texture_size");
	program->clip_size_uniform_location = glGetUniformLocation(program->base_data.program_id, "clip_size");
	program->reverse_half_display_size_unifrom_location = glGetUniformLocation(program->base_data.program_id, "reverse_display_half_size");
	program->display_height_uniform_location = glGetUniformLocation(program->base_data.program_id, "display_height");
	program->zoom_uniform_location = glGetUniformLocation(program->base_data.program_id, "zoom");
	program->clip_zoom_uniform_location = glGetUniformLocation(program->base_data.program_id, "clip_zoom");
	program->clip_rotate_uniform_location = glGetUniformLocation(program->base_data.program_id, "clip_rotate");
	program->rotate_uniform_location = glGetUniformLocation(program->base_data.program_id, "rotate");

	program->color_uniform_location = glGetUniformLocation(program->base_data.program_id, "color");
	program->texture_uniform_location = glGetUniformLocation(program->base_data.program_id, "draw_texture");
	program->clip_texture_uniform_location = glGetUniformLocation(program->base_data.program_id, "clip");
}

/*
 InitializeClipDrowProgram関数
 画像でクリッピングして描画するシェーダープログラムの初期化
 引数
 program		: シェーダーを管理するデータ
 display_width	: 描画領域の幅
 display_height	: 描画領域の高さ
 返り値
	成功:TRUE	失敗:FALSE
*/
int InitializeClipDrawProgram(
	CLIP_DRAW_PROGRAM* program,
	float display_width,
	float display_height
)
{
	// 頂点シェーダーのソースコード
	const char vertex_shader[] =
		"#if __VERSION__ < 130\n"
		"#define in attribute\n"
		"#define out varying\n"
		"#endif\n"
		"in vec3 in_position;\n"
		"in vec2 in_texture_coordinate;\n"
		"out vec2 out_texture_coordinate;\n"
		"out vec2 out_clip_coordinate;\n"
		"uniform vec2 position;\n"
		"uniform vec4 texture_position;\n"
		"uniform vec4 clip_position;\n"
		"uniform vec2 texture_size;\n"
		"uniform vec2 clip_size;\n"
		"uniform vec2 display_half_size;\n"
		"uniform vec2 reverse_display_half_size;\n"
		"uniform float display_height;\n"
		"uniform float zoom;\n"
		"uniform float clip_zoom;\n"
		"uniform float rotate;\n"
		"uniform float clip_rotate;\n"
		"void main()\n"
		"{\n"
			"\tfloat x;\n"
			"\tfloat y;\n"
			"\tfloat width;\n"
			"\tfloat height;\n"
			"\tfloat rotate_x;\n"
			"\tfloat rotate_y;\n"
			"\tfloat div_x;\n"
			"\tfloat div_y;\n"
			"\tfloat sin_value;\n"
			"\tfloat cos_value;\n"
			"\tfloat clip_width;\n"
			"\tfloat clip_height;\n"
			"\tfloat clip_zoom_x;\n"
			"\tfloat clip_zoom_y;\n"
			"\tfloat move_x;\n"
			"\tfloat move_y;\n"
			"\tdiv_x = 1.0 / texture_size.x, div_y = 1.0 / texture_size.y;\n"
			"\twidth = texture_position.z - texture_position.x;\n"
			"\theight = texture_position.w - texture_position.y;\n"
			"\tclip_width = (clip_position.z - clip_position.x);\n"
			"\tclip_height = (clip_position.w - clip_position.y);\n"
			"\tclip_zoom_x = (width * zoom) / (clip_width * clip_zoom);\n"
			"\tclip_zoom_y = (height * zoom) / (clip_height * clip_zoom);\n"
			"\tx = (- (width * 0.5) + width * in_position.x) * zoom;\n"
			"\ty = (- (height * 0.5) + height * in_position.y) * zoom;\n"
			"\tsin_value = sin(rotate);\n"
			"\tcos_value = cos(rotate);\n"
			"\trotate_x = x * cos_value + y * sin_value;\n"
			"\trotate_y = - x * sin_value + y * cos_value;\n"
			"\tgl_Position = vec4((rotate_x + width * 0.5 + (clip_zoom_x - 1) * clip_width * 0.5 * (clip_zoom / zoom) + (zoom - clip_zoom) * (width - clip_width) * 0.5 + position.x) * reverse_display_half_size.x - 1,\n"
				"\t\t(display_height - rotate_y - height * 0.5 - (clip_zoom_y - 1) * clip_height * 0.5 - (zoom - clip_zoom) * clip_height * clip_zoom_y * 0.5 - position.y) * reverse_display_half_size.y - 1, in_position.z, 1);\n"
			"\tout_texture_coordinate = vec2(texture_position.x * div_x + in_texture_coordinate.x * width * div_x,\n"
				"\t\ttexture_position.y * div_y + in_texture_coordinate.y * height * div_y);\n"
			"\tdiv_x = 1.0 / clip_size.x, div_y = 1.0 / clip_size.y;\n"
			"\tmove_x = (clip_position.x + clip_position.z) * 0.5, move_y = (clip_position.y + clip_position.w) * 0.5;\n"
			"\tx = - move_x * div_x + (clip_width * div_x * in_texture_coordinate.x * clip_zoom_x);\n"
			"\ty = - move_y * div_y + (clip_height * div_y * in_texture_coordinate.y * clip_zoom_y);\n"
			"\tsin_value = sin(clip_rotate);\n"
			"\tcos_value = cos(clip_rotate);\n"
			"\trotate_x = x * cos_value + y * sin_value;\n"
			"\trotate_y = - x * sin_value + y * cos_value;\n"
			"\tout_clip_coordinate = vec2(rotate_x + move_x * div_x, rotate_y + move_y * div_y);\n"
		"}";
	const char fragment_shader[] =
		"#if __VERSION__ < 130\n"
		"#define in varying\n"
		"#define out_pixel_color gl_FragColor\n"
		"#define texture(samp, uv) texture2D((samp), (uv))\n"
		"#else\n"
		"out vec4 out_pixel_color;\n"
		"#endif\n"
		"in vec2 out_texture_coordinate;\n"
		"in vec2 out_clip_coordinate;\n"
		"uniform vec4 color;\n"
		"uniform sampler2D draw_texture;\n"
		"uniform sampler2D clip;\n"
		"void main()\n"
		"{\n"
			"\tvec4 texture_color = texture(draw_texture, out_texture_coordinate);\n"
			"\tout_pixel_color = color * mix(vec4(0), texture(draw_texture, out_texture_coordinate), texture(clip, out_clip_coordinate).a);\n"
		"}\n";
	// 頂点バッファ
	const DRAW_SQUARE_VERTEX vertices[4] =
	{
		{{0, 0, 0}, {0, 0}},
		{{1, 0, 0}, {1, 0}},
		{{1, 1, 0}, {1, 1}},
		{{0, 1, 0}, {0, 1}}
	};
	// インデックス
	const unsigned char indices[4] = {0, 1, 2, 3};

	// シェーダープログラムを設定
	if(InitializeShaderProgram(&program->base_data, vertex_shader, fragment_shader,
			"in_position", SHADER_ATTRIBUTE_VERTEX,
			"in_texture_coordinate", SHADER_ATTRIBUTE_TEXTURE_COORD,
			NULL
		)
		== FALSE
	)
	{
		return FALSE;
	}

	// uniform属性の変数識別IDを取得
	ClipDrawProgramSetUniformLocation(program);

	// uniform変数の初期値を設定する
	glUseProgram(program->base_data.program_id);
	glUniform2f(program->reverse_half_display_size_unifrom_location,
		1 / (display_width * 0.5f), 1 / (display_height * 0.5f));
	glUniform1f(program->display_height_uniform_location, (GLfloat)display_height);
	glUniform1f(program->zoom_uniform_location, 1.0f);
	glUniform1f(program->clip_zoom_uniform_location, 1.0f);
	glUniform1f(program->rotate_uniform_location, 0.0f);
	glUniform1f(program->clip_rotate_uniform_location, 0.0f);
	glUniform4f(program->color_uniform_location, 1.0f, 1.0f, 1.0f, 1.0f);
	glUseProgram(0);

	// 頂点バッファを初期化
	InitializeVertexBuffer(
		&program->vertex_buffer,
		sizeof(vertices),
		(void*)vertices,
		sizeof(indices),
		(void*)indices,
		sizeof(*vertices),
		3, SHADER_ATTRIBUTE_VERTEX, GL_FLOAT, GL_FALSE, NULL,
		2, SHADER_ATTRIBUTE_TEXTURE_COORD, GL_FLOAT, GL_FALSE, offsetof(DRAW_SQUARE_VERTEX, texture_coordinate),
		0
	);

	// 頂点バッファにデータを渡す
	glBindBuffer(GL_ARRAY_BUFFER, program->vertex_buffer.vertex_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, program->vertex_buffer.index_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return TRUE;
}

/*
 InitializeDisplayPrograms関数
 描画に使うシェーダーの全てを初期化する
 引数
 programs	: シェーダー全体を管理するデータ
*/
void InitializeDisplayPrograms(DISPLAY_PROGRAMS* programs)
{
	InitializeDrawSquareProgram(&programs->draw_square,
		DISPLAY_WIDTH, DISPLAY_HEIGHT);
	InitializeClipDrawProgram(&programs->clip_draw,
		DISPLAY_WIDTH, DISPLAY_HEIGHT);
}

#ifdef __cplusplus
}
#endif
 