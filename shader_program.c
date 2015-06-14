#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include "shader_program.h"
#include "configure.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 AddShaderSource�֐�
 GLSL�̃v���O�����I�u�W�F�N�g�ɃV�F�[�_�[�I�u�W�F�N�g��ǉ�
 ����
 program_id	: �v���O�����I�u�W�F�N�g
 source		: GLSL�̃V�F�[�_�[�\�[�X�R�[�h
 type		: �V�F�[�_�[�̃^�C�v(���_�V�F�[�_�[ or �t���O�����g�V�F�[�_�[)
 �Ԃ�l
	����:TRUE	���s:FALSE
*/
static int AddShaderSource(GLuint program_id, const char* source, GLenum type)
{
	GLint compiled;		// �\�[�X�R�[�h�̃R���p�C���̐���/���s
	GLuint shader;		// �V�F�[�_�[�I�u�W�F�N�g
	
	// �V�F�[�_�[�I�u�W�F�N�g�𐶐�
	shader = glCreateShader(type);

	// ���������V�F�[�_�[�I�u�W�F�N�g�Ƀ\�[�X�R�[�h��n��
	glShaderSource(shader, 1, &source, NULL);
	// �n�����\�[�X�R�[�h���R���p�C��
	glCompileShader(shader);
	// �R���p�C���ɐ����������m�F
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	// compiled�̒��g��0�Ȃ�R���p�C�����s
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
 GenerateGlslProgram�֐�
 GLSL�̃v���O�����I�u�W�F�N�g�𐶐����A�V�F�[�_�[���R���p�C������
 ����
 vertex_shader_source	: ���_�V�F�[�_�[�̃\�[�X�R�[�h������
 fragment_shader_source	: �t���O�����g�V�F�[�_�[�̃\�[�X�R�[�h������
 �Ԃ�l
	���������v���O�����I�u�W�F�N�g��ID
*/
GLuint GenerateGlslProgram(
	const char* vertex_shader_source,
	const char* fragment_shader_source
)
{
	GLuint program_id;	// �v���O�����I�u�W�F�N�g��ID

	// �v���O�����I�u�W�F�N�g�̐�����OpenGL�Ɉ˗�
	program_id = glCreateProgram();

	// ���_�V�F�[�_�[��ǉ�
	if(AddShaderSource(program_id, vertex_shader_source, GL_VERTEX_SHADER) == FALSE)
	{
		glDeleteProgram(program_id);
		return 0;
	}

	// �t���O�����g�V�F�[�_�[��ǉ�
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
 ReleaseShaderProgram�֐�
 �V�F�[�_�[�v���O�������J������
 ����
 program	: �V�F�[�_�[�v���O�����̊�{�f�[�^
*/
void ReleaseShaderProgram(SHADER_PROGRAM_BASE* program)
{
	glDeleteProgram(program->program_id);
	program->program_id = 0;
}

/*
 ShaderProgramLink�֐�
 �V�F�[�_�[�v���O�����������N����
 ����
 program	: �V�F�[�_�[�v���O�����̊�{�f�[�^
 �Ԃ�l
	����:TRUE	���s:FALSE
*/
int ShaderProgramLink(SHADER_PROGRAM_BASE* program)
{
	GLint status;	// �����N�̐���/���s���󂯂�

	// �����N���s
	glLinkProgram(program->program_id);
	// �����N�ɐ������������ׂ�
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
 InitializeShaderProgram�֐�
 1����GLSL�̃v���O���}�u���V�F�[�_�[�Ǘ��f�[�^��������
 ����
 program				: �V�F�[�_�[�v���O�����̊�{�f�[�^
 vertex_shader_source	: ���_�V�F�[�_�[�̃\�[�X�R�[�h������
 fragment_shader_source	: �t���O�����g�V�F�[�_�[�̃\�[�X�R�[�h������
 �ϒ�����				: ���_�V�F�[�_�[����in�C���̕ϐ����Ƃ���Ɋ��蓖�Ă�ID
							(const char*�̖��O, GLuint��key�����݂Ɏw��. �Ō��NULL���w��)
 �Ԃ�l
	����:TRUE	���s:FALSE
*/
int InitializeShaderProgram(
	SHADER_PROGRAM_BASE* program,
	const char* vertex_shader_source,
	const char* fragment_shader_source,
	const char* first_attribute,
	...
)
{
	const char *attribute_name;	// ���͕ϐ���
	va_list list;				// �ϒ������̃��X�g

	(void)memset(program, 0, sizeof(*program));

	if((program->program_id = GenerateGlslProgram(
		vertex_shader_source, fragment_shader_source)) == 0)
	{
		return FALSE;
	}

	// ���͕ϐ��̖��O��ID�����ѕt����
	attribute_name = first_attribute;
	va_start(list, first_attribute);
	while(attribute_name != NULL)
	{
		GLuint key = va_arg(list, GLuint);
		glBindAttribLocation(program->program_id, key, attribute_name);
		attribute_name = va_arg(list, const char*);
	}
	va_end(list);

	// �R���p�C�������V�F�[�_�[�������N
	if(ShaderProgramLink(program) == FALSE)
	{
		ReleaseShaderProgram(program);
		return FALSE;
	}

	return TRUE;
}

/*
 DrawSquareProgramSetUniformLocation�֐�
 �V�F�[�_�[�I�u�W�F�N�g����uniform�����̕ϐ���ID���擾����
 ����
 program	: �V�F�[�_�[���Ǘ�����f�[�^
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
 InitializeDrawSquareProgram�֐�
 �摜�S�̂��g���ĕ`�悷��V�F�[�_�[�v���O�����̏�����
 ����
 program		: �V�F�[�_�[���Ǘ�����f�[�^
 display_width	: �`��̈�̕�
 display_height	: �`��̈�̍���
 �Ԃ�l
	����:TRUE	���s:FALSE
*/
int InitializeDrawSquareProgram(
	DRAW_SQUARE_PROGRAM* program,
	float display_width,
	float display_height
)
{
	// ���_�V�F�[�_�[�̃\�[�X�R�[�h
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
	// ���_�o�b�t�@
	const DRAW_SQUARE_VERTEX vertices[4] =
	{
		{{0, 0, 0}, {0, 0}},
		{{1, 0, 0}, {1, 0}},
		{{1, 1, 0}, {1, 1}},
		{{0, 1, 0}, {0, 1}}
	};
	// �C���f�b�N�X
	const unsigned char indices[4] = {0, 1, 2, 3};

	// �V�F�[�_�[�v���O������ݒ�
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

	// uniform�����̕ϐ�����ID���擾
	DrawSquareProgramSetUniformLocation(program);

	// uniform�ϐ��̏����l��ݒ肷��
	glUseProgram(program->base_data.program_id);
	glUniform2f(program->reverse_half_display_size_unifrom_location,
		1 / (display_width * 0.5f), 1 / (display_height * 0.5f));
	glUniform1f(program->display_height_uniform_location, (GLfloat)display_height);
	glUniform1f(program->zoom_uniform_location, 1.0f);
	glUniform1f(program->rotate_uniform_location, 0.0f);
	glUniform4f(program->color_uniform_location, 1.0f, 1.0f, 1.0f, 1.0f);
	glUseProgram(0);

	// ���_�o�b�t�@��������
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

	// ���_�o�b�t�@�Ƀf�[�^��n��
	glBindBuffer(GL_ARRAY_BUFFER, program->vertex_buffer.vertex_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, program->vertex_buffer.index_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return TRUE;
}

/*
 ClipDrawProgramSetUniformLocation�֐�
 �V�F�[�_�[�I�u�W�F�N�g����uniform�����̕ϐ���ID���擾����
 ����
 program	: �V�F�[�_�[���Ǘ�����f�[�^
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
 InitializeClipDrowProgram�֐�
 �摜�ŃN���b�s���O���ĕ`�悷��V�F�[�_�[�v���O�����̏�����
 ����
 program		: �V�F�[�_�[���Ǘ�����f�[�^
 display_width	: �`��̈�̕�
 display_height	: �`��̈�̍���
 �Ԃ�l
	����:TRUE	���s:FALSE
*/
int InitializeClipDrawProgram(
	CLIP_DRAW_PROGRAM* program,
	float display_width,
	float display_height
)
{
	// ���_�V�F�[�_�[�̃\�[�X�R�[�h
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
	// ���_�o�b�t�@
	const DRAW_SQUARE_VERTEX vertices[4] =
	{
		{{0, 0, 0}, {0, 0}},
		{{1, 0, 0}, {1, 0}},
		{{1, 1, 0}, {1, 1}},
		{{0, 1, 0}, {0, 1}}
	};
	// �C���f�b�N�X
	const unsigned char indices[4] = {0, 1, 2, 3};

	// �V�F�[�_�[�v���O������ݒ�
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

	// uniform�����̕ϐ�����ID���擾
	ClipDrawProgramSetUniformLocation(program);

	// uniform�ϐ��̏����l��ݒ肷��
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

	// ���_�o�b�t�@��������
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

	// ���_�o�b�t�@�Ƀf�[�^��n��
	glBindBuffer(GL_ARRAY_BUFFER, program->vertex_buffer.vertex_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, program->vertex_buffer.index_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return TRUE;
}

/*
 InitializeDisplayPrograms�֐�
 �`��Ɏg���V�F�[�_�[�̑S�Ă�����������
 ����
 programs	: �V�F�[�_�[�S�̂��Ǘ�����f�[�^
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
 