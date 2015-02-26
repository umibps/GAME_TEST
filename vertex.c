#include <string.h>
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
 index_size		: �C���f�b�N�X�o�b�t�@�̃T�C�Y
 data_stride	: ���_�f�[�^�\���̂̃o�C�g��
 vertex_key		: GLSL�ɓn�������_���W�̎���ID
 vertex_offset	: ���_���W�f�[�^�̈ʒu
 texture_key	: GLSL�ɓn�����e�N�X�`�����W�̎���ID
 texture_offset	: �e�N�X�`�����W�f�[�^�̈ʒu
 color_key		: GLSL�ɓn�����F�f�[�^�̎���ID
 color_offset	: �F�f�[�^�̈ʒu
*/
void InitializeVertexBuffer(
	VERTEX_BUFFER* buffer,
	size_t vertex_size,
	size_t index_size,
	size_t data_stride,
	int vertex_key,
	void* vertex_offset,
	int texture_key,
	void* texture_offset,
	int color_key,
	void* color_offset
)
{
	(void)memset(buffer, 0, sizeof(*buffer));

	// �f�[�^�z�u���L������o�b�t�@�𐶐�
	glGenVertexArrays(1, &buffer->vertex_array);

	// ���_�o�b�t�@�𐶐�
	glGenBuffers(1, &buffer->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// �C���f�b�N�X�o�b�t�@�𐶐�
	glGenBuffers(1, &buffer->index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// �f�[�^�̔z�u�L�����J�n
	glBindVertexArray(buffer->vertex_array);

	// ���_�o�b�t�@�̔z�u��ݒ�
		// �����������_�o�b�t�@���Ăяo��
	glBindBuffer(GL_ARRAY_BUFFER, buffer->vertex_buffer);

	// ���_�̃f�[�^�ʒu��ݒ�
	glVertexAttribPointer(vertex_key, 3, GL_FLOAT, GL_FALSE, data_stride, vertex_offset);
	if(texture_offset != NULL)
	{
		glVertexAttribPointer(texture_key, 2, GL_FLOAT, GL_FALSE, data_stride, texture_offset);
	}
	if(color_offset != NULL)
	{
		glVertexAttribPointer(color_key, 4, GL_UNSIGNED_BYTE, GL_TRUE, data_stride, color_offset);
	}
	glEnableVertexAttribArray(vertex_key);
	if(texture_offset != NULL)
	{
		glEnableVertexAttribArray(texture_key);
	}
	if(color_offset != NULL)
	{
		glEnableVertexAttribArray(color_key);
	}

	// �f�[�^�z�u�̋L���I��
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// �f�[�^�z�u�𖳌������Ă���
	glDisableVertexAttribArray(vertex_key);
	if(texture_offset != NULL)
	{
		glDisableVertexAttribArray(texture_key);
	}
	if(color_offset != NULL)
	{
		glDisableVertexAttribArray(color_key);
	}
}

#ifdef __cplusplus
}
#endif
