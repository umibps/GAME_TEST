#ifndef _INCLUDED_SHADER_PROGRAM_H_
#define _INCLUDED_SHADER_PROGRAM_H_

#include <GL/glew.h>
#include <GL/glut.h>
#include "types.h"

/*
*/
typedef enum _eSHADER_ATTRIBUTE
{
	SHADER_ATTRIBUTE_VERTEX,
	SHADER_ATTRIBUTE_TEXTURE_COORD,
	SHADER_ATTRIBUTE_COLOR
} eSHADER_ATTRIBUTE;

/*
 SHADER_PROGRAM_BASE�\����
 GLSL�̊�{�f�[�^
*/
typedef struct _SHADER_PROGRAM_BASE
{
	GLuint program_id;
} SHADER_PROGRAM_BASE;

/*
*/
typedef struct _DRAW_SQUARE_PROGRAM
{
	SHADER_PROGRAM_BASE base_data;			// �V�F�[�_�[�v���O�����̊�{�f�[�^
	GLint texture_size_uniform_location;	// �e�N�X�`���摜�̃T�C�Y������
											// �V�F�[�_�[�v���O�����̕ϐ�����ID
	GLint display_size_unifrom_location;	// ��ʃT�C�Y������
											// �V�F�[�_�[�v���O�����̕ϐ�����ID
	GLint zoom_uniform_location;			// X�����y��Y�����̊g�嗦������
											// �V�F�[�_�[�v���O�����̕ϐ�����ID
	GLint rotate_uniform_location;			// �摜�̉�]�p������
											// �V�F�[�_�[�v���O�����̕ϐ�����ID
} DRAW_SQUARE_PROGRAM;

#ifdef __cplusplus
extern "C" {
#endif

/*
 GenerateGlslProgram�֐�
 GLSL�̃v���O�����I�u�W�F�N�g�𐶐����A�V�F�[�_�[���R���p�C������
 ����
 vertex_shader_source	: ���_�V�F�[�_�[�̃\�[�X�R�[�h������
 fragment_shader_source	: �t���O�����g�V�F�[�_�[�̃\�[�X�R�[�h������
 �Ԃ�l
	���������v���O�����I�u�W�F�N�g��ID
*/
EXTERN GLuint GenerateGlslProgram(
	const char* vertex_shader_source,
	const char* fragment_shader_source
);

/*
 ReleaseShaderProgram�֐�
 �V�F�[�_�[�v���O�������J������
 ����
 program	: �V�F�[�_�[�v���O�����̊�{�f�[�^
*/
EXTERN void ReleaseShaderProgram(SHADER_PROGRAM_BASE* program);

/*
 ShaderProgramLink�֐�
 �V�F�[�_�[�v���O�����������N����
 ����
 program	: �V�F�[�_�[�v���O�����̊�{�f�[�^
 �Ԃ�l
	����:TRUE	���s:FALSE
*/
EXTERN int ShaderProgramLink(SHADER_PROGRAM_BASE* program);

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
EXTERN int InitializeShaderProgram(
	SHADER_PROGRAM_BASE* program,
	const char* vertex_shader_source,
	const char* fragment_shader_source,
	const char* first_attribute,
	...
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SHADER_PROGRAM_H_
