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
