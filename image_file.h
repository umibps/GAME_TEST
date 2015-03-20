#ifndef _INCLUDED_IMAGE_FILE_H_
#define _INCLUDED_IMAGE_FILE_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 ReadPngStream�֐�
 PNG�C���[�W�f�[�^��ǂݍ��݃f�R�[�h����
 ����
 stream		: �f�[�^�X�g���[��
 read_func	: �ǂݍ��݂Ɏg�p����֐��|�C���^
 width		: �摜�̕����󂯂�ϐ��̃A�h���X
 height		: �摜�̍������󂯂�ϐ��̃A�h���X
 channel	: �摜�̃`�����l�������󂯂�ϐ��̃A�h���X
 �Ԃ�l
	����:�s�N�Z���f�[�^	���s:NULL
*/
EXTERN uint8* ReadPngStream(
	void* stream,
	size_t(*read_func)(void*, size_t, size_t, void*),
	int* width,
	int* height,
	int* channel
);

/*
 WritePngStream�֐�
 PNG�C���[�W�f�[�^���G���R�[�h���ď����o��
 ����
 stream			: �f�[�^�X�g���[��
 write_func		: �ǂݍ��݂Ɏg�p����֐��|�C���^
 flush_func		: �o�b�t�@�N���A�p�̊֐��|�C���^(NULL�w��\)
 pixels			: �摜�̃s�N�Z���f�[�^
 width			: �摜�̕�
 height			: �摜�̍���
 channel		: �摜�̃`�����l����
 compression	: ���k���x��
*/
EXTERN void WritePngStream(
	void* stream,
	size_t(*write_func)(void*, size_t, size_t, void*),
	void(*flush_func)(void*),
	uint8* pixels,
	int width,
	int height,
	int channel,
	int compression
);

/*
 ReadJpegStream�֐�
 ����
 stream		: �f�[�^�X�g���[��
 read_func	: �ǂݍ��݂Ɏg�p����֐��|�C���^
 data_size	: �摜�f�[�^�̃o�C�g��
 width		: �摜�̕����󂯂�ϐ��̃A�h���X
 height		: �摜�̍������󂯂�ϐ��̃A�h���X
 channel	: �摜�̃`�����l�������󂯂�ϐ��̃A�h���X
 �Ԃ�l
	����:�s�N�Z���f�[�^	���s:NULL
*/
EXTERN uint8* ReadJpegStream(
	void* stream,
	size_t(*read_func)(void*, size_t, size_t, void*),
	size_t data_size,
	int* width,
	int* height,
	int* channel
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_IMAGE_FILE_H_
