#include <stdio.h>
#include <stdarg.h>
#include "shader_program.h"

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

	// �R���p�C�������V�F�[�_�[�������N
	if(ShaderProgramLink(program) == FALSE)
	{
		ReleaseShaderProgram(program);
		return FALSE;
	}

	return TRUE;
}

#ifdef __cplusplus
}
#endif
