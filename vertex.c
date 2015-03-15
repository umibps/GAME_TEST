#include <string.h>
#include <stdarg.h>
#include "vertex.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeVertexBuffer�֐�
 ���_�o�b�t�@������������
 ����
 buffer			: ���_�o�b�t�@���Ǘ�����\����
 vertex_size	: ���_�o�b�t�@�̃T�C�Y
 vertex_data	: ���_�o�b�t�@�ɓn���f�[�^(NULL��)
 index_size		: �C���f�b�N�X�o�b�t�@�̃T�C�Y
 index_data		: �C���f�b�N�X�o�b�t�@�ɓn���f�[�^(NULL��)
 data_stride	: ���_�f�[�^�\���̂̃o�C�g��
 �ϒ�����		: �z��̒���(int), �f�[�^��ID(int), �f�[�^�̌^(GLenum),
					�f�[�^�̐��K���L��(GL_TRUE or GL_FALSE), �f�[�^�̃I�t�Z�b�g(size_t)
					�Ŏw�肷�� (�Ō��0)
*/
void InitializeVertexBuffer(
	VERTEX_BUFFER* buffer,
	size_t vertex_size,
	void* vertex_data,
	size_t index_size,
	void* index_data,
	size_t data_stride,
	int first_size,
	...
)
{
	int key;				// �f�[�^��ID
	int size;				// �f�[�^�̔z��
	GLenum type;			// �f�[�^�̌^
	GLboolean normalized;	// ���K���L��
	size_t offset;			// �f�[�^�̈ʒu
	va_list list;			// �ϒ������̃��X�g

	(void)memset(buffer, 0, sizeof(*buffer));

	// �f�[�^�z�u���L������o�b�t�@�𐶐�
	glGenVertexArrays(1, &buffer->vertex_array);

	// ���_�o�b�t�@�𐶐�
	glGenBuffers(1, &buffer->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_size, vertex_data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// �C���f�b�N�X�o�b�t�@�𐶐�
	glGenBuffers(1, &buffer->index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size, index_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// �f�[�^�̔z�u�L�����J�n
	glBindVertexArray(buffer->vertex_array);

	// ���_�o�b�t�@�̔z�u��ݒ�
		// �����������_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@���Ăяo��
	glBindBuffer(GL_ARRAY_BUFFER, buffer->vertex_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->index_buffer);

	// ���_�̃f�[�^�ʒu��ݒ�
	va_start(list, first_size);
	VaListAttributePointer(list, first_size, data_stride);
	va_end(list);

	// �f�[�^�z�u�̋L���I��
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// �f�[�^�z�u�𖳌������Ă���
	va_start(list, first_size);
	size = first_size;
	while(size > 0)
	{
		key = va_arg(list, int);
		type = va_arg(list, GLenum);
		normalized = va_arg(list, GLboolean);
		offset = va_arg(list, size_t);
		glDisableVertexAttribArray(key);
		size = va_arg(list, int);
	}
	va_end(list);
}

/*
 InitializeVertexArray�֐�
 ���_�o�b�t�@���g�p���Ȃ��ꍇ�̃f�[�^�z�u��ݒ肷��
 ����
 vertex_array	: �f�[�^�z�u���L�����钸�_�z��I�u�W�F�N�g
 vertex_size	: ���_�o�b�t�@�̃T�C�Y
 index_size		: �C���f�b�N�X�o�b�t�@�̃T�C�Y
 data_stride	: ���_�f�[�^�\���̂̃o�C�g��
 �ϒ�����		: �z��̒���(int), �f�[�^��ID(int), �f�[�^�̌^(GLenum),
					�f�[�^�̐��K���L��(GL_TRUE or GL_FALSE), �f�[�^�̃I�t�Z�b�g(size_t)
					�Ŏw�肷�� (�Ō��0)
*/
void InitializeVertexArray(
	GLuint* vertex_array,
	size_t data_stride,
	int first_size,
	...
)
{
	int key;				// �f�[�^��ID
	int size;				// �f�[�^�̔z��
	GLenum type;			// �f�[�^�̌^
	GLboolean normalized;	// ���K���L��
	size_t offset;			// �f�[�^�̈ʒu
	va_list list;			// �ϒ������̃��X�g

	// ���_�z��I�u�W�F�N�g���쐬
	glGenVertexArrays(1, vertex_array);

	// �f�[�^�z�u�̋L���J�n
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(*vertex_array);

	// ���_�̃f�[�^�ʒu��ݒ�
	va_start(list, first_size);
	VaListAttributePointer(list, first_size, data_stride);
	va_end(list);

	// �f�[�^�z�u�̋L���I��
	glBindVertexArray(0);

	// �f�[�^�z�u�𖳌������Ă���
	va_start(list, first_size);
	size = first_size;
	while(size > 0)
	{
		key = va_arg(list, int);
		type = va_arg(list, GLenum);
		normalized = va_arg(list, GLboolean);
		offset = va_arg(list, size_t);
		glDisableVertexAttribArray(key);
		size = va_arg(list, int);
	}
	va_end(list);
}

/*
 VaListAttributePointer�֐�
 va_list����OpenGL�ɓn���f�[�^�̔z�u��ݒ肷��
 ����
 list		: va_list(���e��SetAttributePointer�֐����Q��)
 stride		: ���_�f�[�^1���̃o�C�g��
 first_size	: �ŏ��̃f�[�^�̔z��
*/
void VaListAttributePointer(va_list list, int first_size, size_t stride)
{
	int key;				// �f�[�^��ID
	int size;				// �f�[�^�̔z��
	GLenum type;			// �f�[�^�̌^
	GLboolean normalized;	// ���K���L��
	size_t offset;			// �f�[�^�̈ʒu

	// �n���ꂽ�������g���f�[�^�̔z�u��ݒ�
	size = first_size;
	while(size > 0)
	{
		key = va_arg(list, int);
		type = va_arg(list, GLenum);
		normalized = va_arg(list, GLboolean);
		offset = va_arg(list, size_t);
		glVertexAttribPointer(key, size, type,
			normalized, stride, (const void*)offset);
		glEnableVertexAttribArray(key);
		size = va_arg(list, int);
	}
}

/*
 SetAttributePointer�֐�
 OpenGL�ɓn�����_�f�[�^�̔z�u��ݒ肷��
 ����
 stride		: ���_�f�[�^1���̃o�C�g��
 �ϒ�����	: �z��̒���(int), �f�[�^��ID(int), �f�[�^�̌^(GLenum),
				�f�[�^�̐��K���L��(GL_TRUE or GL_FALSE), �f�[�^�̃I�t�Z�b�g(size_t)
				�Ŏw�肷�� (�Ō��0)
*/
void SetAttributePointer(
	size_t stride,
	int first_size,
	...
)
{
	va_list list;

	va_start(list, first_size);
	
	VaListAttributePointer(list, first_size, stride);

	va_end(list);
}

#ifdef __cplusplus
}
#endif
