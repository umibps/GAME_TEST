#ifndef _INCLUDED_UTILS_H_
#define _INCLUDED_UTILS_H_

#include <stdio.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 StringCompareIgnoreCase�֐�
 �啶��/�������𖳎����ĕ�������r
 ����
 str1	: ��r���镶���񂻂�1
 str2	: ��r���镶���񂻂�2
 �Ԃ�l
	str1 - str2
*/
EXTERN int StringCompareIgnoreCase(const char* str1, const char* str2);

/*
 FileOpen�֐�
 �t�@�C�����J��
 ����
 path		: �t�@�C���̃p�X
 mode		: �J�����[�h(fopen�֐��ɏ���)
 user_data	: �v���O���}�[�����R�Ɏg����f�[�^
 �Ԃ�l
	����:�t�@�C���|�C���^	���s:NULL
*/
EXTERN FILE* FileOpen(const char* path, const char* mode, void* user_data);

/*
 FileClose�֐�
 �t�@�C�������
 ����
 fp			: �t�@�C���|�C���^
 user_data	: �v���O���}�[�����R�Ɏg����f�[�^
 �Ԃ�l
	���0
*/
EXTERN int FileClose(FILE* fp, void* user_data);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_UTILS_H_
