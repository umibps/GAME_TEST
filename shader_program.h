#ifndef _INCLUDED_SHADER_PROGRAM_H_
#define _INCLUDED_SHADER_PROGRAM_H_

#include <GL/glew.h>
#include <GL/glut.h>
#include "types.h"
#include "vertex.h"

/*
 eSHADER_ATTRIBUTE�񋓑�
 ���_�o�b�t�@�̑����萔
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
 DRAW_SQUARE_PROGRAM�\����
 �摜�S�̂��g���ĕ`�悷��ꍇ�̃f�[�^
*/
typedef struct _DRAW_SQUARE_PROGRAM
{
	SHADER_PROGRAM_BASE base_data;						// �V�F�[�_�[�v���O�����̊�{�f�[�^
	VERTEX_BUFFER vertex_buffer;						// (0, 1)�݂̂̒��_�o�b�t�@
	GLint position_uniform_location;					// �`�悷����W������
														// ���_�V�F�[�_�[�v���O�����̕ϐ�����ID
	GLint texture_size_uniform_location;				// �e�N�X�`���摜�̃T�C�Y������
														// ���_�V�F�[�_�[�v���O�����̕ϐ�����ID
	GLint reverse_half_display_size_unifrom_location;	// �摜�T�C�Y�Ǝw����W����OpenGL�ł̍��W���v�Z����
														// ���_�V�F�[�_�[�v���O�����̕ϐ�����ID
	GLint display_height_uniform_location;				// �`��̈�̍������w�肷��
														// ���_�V�F�[�_�[�v���O�����̕ϐ�����ID
	GLint zoom_uniform_location;						// X�����y��Y�����̊g�嗦������
														// ���_�V�F�[�_�[�v���O�����̕ϐ�����ID
	GLint rotate_uniform_location;						// �摜�̉�]�p������
														// ���_�V�F�[�_�[�v���O�����̕ϐ�����ID
	GLint color_uniform_location;						// �摜�̐F���w�肷��
														// �t���O�����g�V�F�[�_�[�v���O�����̕ϐ�����ID
	GLint texture_uniform_location;						// �g�p����e�N�X�`�����j�b�g��ID���w�肷��
														// �t���O�����g�V�F�[�_�[�v���O�����̕ϐ�����ID
} DRAW_SQUARE_PROGRAM;

/*
 DISPLAY_PROGRAMS�\����
 �g�p����V�F�[�_�[�v���O�����̏W����
*/
typedef struct _DISPLAY_PROGRAMS
{
	DRAW_SQUARE_PROGRAM draw_square;
} DISPLAY_PROGRAMS;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeDisplayPrograms�֐�
 �`��Ɏg���V�F�[�_�[�̑S�Ă�����������
 ����
 programs	: �V�F�[�_�[�S�̂��Ǘ�����f�[�^
*/
EXTERN void InitializeDisplayPrograms(DISPLAY_PROGRAMS* programs);

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
EXTERN int InitializeDrawSquareProgram(
	DRAW_SQUARE_PROGRAM* program,
	float display_width,
	float display_height
);

/*
 DrawSquareProgramSetUniformLocation�֐�
 �V�F�[�_�[�I�u�W�F�N�g����uniform�����̕ϐ���ID���擾����
 ����
 program	: �V�F�[�_�[���Ǘ�����f�[�^
*/
EXTERN void DrawSquareProgramSetUniformLocation(DRAW_SQUARE_PROGRAM* program);

/*
 InitializeDisplayPrograms�֐�
 �`��Ɏg���V�F�[�_�[�̑S�Ă�����������
 ����
 programs	: �V�F�[�_�[�S�̂��Ǘ�����f�[�^
*/
EXTERN void InitializeDisplayPrograms(DISPLAY_PROGRAMS* programs);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SHADER_PROGRAM_H_
