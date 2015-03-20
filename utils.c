#include <stdio.h>
#include <ctype.h>

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
int StringCompareIgnoreCase(const char* str1, const char* str2)
{
	int ret;

	while((ret = toupper(*str1) - toupper(*str2)) == 0)
	{
		str1++, str2++;
		if(*str1 == '\0')
		{
			return 0;
		}
	}

	return ret;
}

/*
 GetNextUtf8Character�֐�
 ����UTF8�̕������擾����
 ����
 str	: ���̕������擾������������
 �Ԃ�l
	���̕����̃A�h���X
*/
const char* GetNextUtf8Character(const char* str)
{
	static const unsigned char skips[256] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
	};

	return str + skips[((unsigned char)*str)];
}

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
FILE* FileOpen(const char* path, const char* mode, void* user_data)
{
	return fopen(path, mode);
}

/*
 FileClose�֐�
 �t�@�C�������
 ����
 fp			: �t�@�C���|�C���^
 user_data	: �v���O���}�[�����R�Ɏg����f�[�^
 �Ԃ�l
	���0
*/
int FileClose(FILE* fp, void* user_data)
{
	(void)fclose(fp);

	return 0;
}

#ifdef __cplusplus
}
#endif
