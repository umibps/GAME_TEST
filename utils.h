#ifndef _INCLUDED_UTILS_H_
#define _INCLUDED_UTILS_H_

#include <stdio.h>
#include <stdarg.h>
#include "types.h"

/*
 POINTER_ARRAY�\����
 �ϒ��̃|�C���^�^�z��
*/
typedef struct _POINTER_ARRAY
{
	void **buffer;					// �z��̎���
	size_t buffer_size;				// ���݂̔z��̃T�C�Y
	size_t num_data;				// ���ݕێ����Ă���f�[�^�̐�
	size_t block_size;				// �z��g�厞�̍X�V��
	void (*delete_func)(void*);		// �f�[�^�폜���Ɏg���֐��|�C���^
} POINTER_ARRAY;

/*
 PRIORITY_ARRAY_ITEM�\����
 �D��x�t���z��̂ЂƂ̍���
*/
typedef struct _PRIORITY_ARRAY_ITEM
{
	void *data;				// �ėp�|�C���^
	unsigned int priority;	// �D��x
} PRIORITY_ARRAY_ITEM;

/*
 PRIORITY_ARRAY�\����
 �D��x�t���z��
*/
typedef struct _PRIORITY_ARRAY
{
	PRIORITY_ARRAY_ITEM *buffer;	// �҂��s��̎���
	size_t buffer_size;				// ���݂̑҂��s��̃T�C�Y
	size_t num_data;				// ���ݕێ����Ă��鍀�ڂ̐�
	size_t block_size;				// �҂��s��̃T�C�Y�g�厞�̍X�V��
	void (*delete_func)(void*);		// �f�[�^�폜���Ɏg���֐��|�C���^
} PRIORITY_ARRAY;

/*
 HASH_TABLE_ITEM�\����
 ��������L�[�Ƃ���n�b�V���e�[�u����1�̍��ڂ̃f�[�^
*/
typedef struct _STRING_HASH_TABLE_ITEM
{
	void *data;					// �ėp�|�C���^
	unsigned int hash_value;	// �n�b�V���l
	const char *key;			// �L�[�̕�����
} STRING_HASH_TABLE_ITEM;

/*
 STRING_HASH_TABLE�\����
 ��������L�[�Ƃ���n�b�V���e�[�u��
*/
typedef struct _STRING_HASH_TABLE
{
	STRING_HASH_TABLE_ITEM *buffer;	// �n�b�V���e�[�u���̎���
	unsigned int buffer_size;		// ���݂̃n�b�V���e�[�u���̃T�C�Y
	unsigned int num_data;			// ���ݕێ����Ă��鍀�ڂ̐�
	unsigned int block_size;		// �n�b�V���e�[�u���̃T�C�Y�g�厞�̍X�V��
	void (*delete_func)(void*);		// �f�[�^�폜���Ɏg���֐��|�C���^
} STRING_HASH_TABLE;

/*
 RGBA�}�N���֐�
 �ԗΐ��l����32�r�b�g���������������쐬
*/
#define RGBA(R, G, B, A) (((R) << 24) | ((G) << 16) | ((B) << 8) | (A))

#ifdef __cplusplus
extern "C" {
#endif

/*
 ErrorMessage�֐�
 �G���[���b�Z�[�W��\������
 ����
 format	: �\�����郁�b�Z�[�W�̃t�H�[�}�b�g
*/
void ErrorMessage(const char* format, ... );

/*
 DummyFunctionNoReturn�֐�
 �������Ȃ��R�[���o�b�N�֐�(�Ԃ�l����)
 ����
 dummy	: �_�~�[�̈���
*/
extern void DummyFunctionNoReturn(void* dummy);

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
 GetNextUtf8Character�֐�
 ����UTF8�̕������擾����
 ����
 str	: ���̕������擾������������
 �Ԃ�l
	���̕����̃A�h���X
*/
EXTERN const char* GetNextUtf8Character(const char* str);

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

/*
 DefaultErrorMessage�֐�
 �f�t�H���g�̃G���[�\���p�֐�
 ����
 message	: �\�����郁�b�Z�[�W
 dummy		: �_�~�[�f�[�^
*/
extern void DefaultErrorMessage(const char* message, void* dummy);

/*
 BinarySearch�֐�
 �񕪒T�������s����
 ����
 search_array	: �T�������z��
 target			: �T�����s���f�[�^
 block_size		: �z���1���̗v�f�̃o�C�g��
 num_blocks		: �z��̃T�C�Y
 compare_func	: �f�[�^��r�p�̊֐�
					(���l:0 ��Ȃ�:1 ���Ȃ�:-1��Ԃ�)
 user_data		: compare_func�̑�3�����ɓ��郆�[�U�[�f�[�^
 �Ԃ�l
	���������v�f��ID (������Ȃ��ꍇ�͍ł��߂����̕���)
*/
EXTERN int BinarySearch(
	void* search_array,
	void* target,
	int block_size,
	int num_blocks,
	int(*compare_func)(void*, void*, void*),
	void* user_data
);

/*
 InitializePointerArray�֐�
 �ϒ��̃|�C���^�^�z�������������
 ����
 pointer_array	: ����������ϒ��|�C���^�^�z��
 block_size		: �z��̃o�b�t�@�T�C�Y�g�厞�̍X�V��
 delete_func	: �f�[�^�폜���Ɏg���֐��|�C���^
*/
EXTERN void InitializePointerArray(
	POINTER_ARRAY* pointer_array,
	size_t block_size,
	void (*delete_func)(void*)
);

/*
 ReleasePointerArray�֐�
 �ϒ��|�C���^�^�z����J������
 ����
 pointer_array	: �ϒ��|�C���^�^�z��
*/
EXTERN void ReleasePointerArray(POINTER_ARRAY* pointer_array);

/*
 PointerArrayAppend�֐�
 �ϒ��|�C���^�^�z��Ƀf�[�^��ǉ�����
 ����
 pointer_array	: �ϒ��|�C���^�^�z��
 data			: �ǉ�����f�[�^
*/
EXTERN void PointerArrayAppend(POINTER_ARRAY* pointer_array, void* data);

/*
 InitializePriorityArray�֐�
 �D��x�t���z�������������
 ����
 priority_array	: ����������D��x�t���z��
 block_size		: �o�b�t�@�T�C�Y�g�厞�̍X�V��
 delete_func	: �f�[�^�폜���Ɏg���֐��|�C���^
*/
EXTERN void InitializePriorityArray(
	PRIORITY_ARRAY* priority_array,
	size_t block_size,
	void (*delete_func)(void*)
);

/*
 ReleasePriorityArray�֐�
 �D��x�t���z����J������
 ����
 priority_array	: �D��x�t���z��
*/
EXTERN void ReleasePriorityArray(PRIORITY_ARRAY* priority_array);

/*
 PriorityArrayAppend�֐�
 �D��x�t���z��Ƀf�[�^��1�ǉ�����
 ����
 priority_array	: �D��x�t���z��
 data			: �ǉ�����f�[�^�̃A�h���X
 priority		: �ǉ�����f�[�^�̗D��x
*/
EXTERN void PriorityArrayAppend(
	PRIORITY_ARRAY* priority_array,
	void* data,
	unsigned int priority
);

/*
 PriorityArrayRemove�֐�
 �D��x�t���z�񂩂�f�[�^��1��菜��
 ����
 priority_array	: �D��x�t���z��
 data			: ��菜���f�[�^
*/
EXTERN void PriorityArrayRemove(
	PRIORITY_ARRAY* priority_array,
	void* data
);

/*
 PriorityArrayReset�֐�
 �D��x�t���z��̓��e�����Z�b�g����
 ����
 priority_array	: �D��x�t���z��
*/
EXTERN void PriorityArrayReset(PRIORITY_ARRAY* priority_array);

/*
 GetStringHashValue�֐�
 ������̃n�b�V���l���擾����
 ����
 key	: �n�b�V���l���擾���镶����
 �Ԃ�l
	�n�b�V���l
*/
EXTERN unsigned int GetStringHashValue(const char* key);

/*
 InitializeStringHashTable�֐�
 ��������L�[�Ƃ���n�b�V���e�[�u��������������
 ����
 hash_table		: �n�b�V���e�[�u�����Ǘ�����f�[�^
 block_size		: �o�b�t�@�T�C�Y���X�V����ۂ̍X�V��
 delete_func	: �f�[�^�폜���Ɏg���֐��|�C���^
*/
EXTERN void InitializeStringHashTable(
	STRING_HASH_TABLE* hash_table,
	size_t block_size,
	void (*delete_func)(void*)
);

/*
 ReleaseStringHashTable�֐�
 ��������L�[�Ƃ���n�b�V���e�[�u�����J������
 ����
 hash_table	: �n�b�V���e�[�u�����Ǘ�����f�[�^
*/
EXTERN void ReleaseStringHashTable(STRING_HASH_TABLE* hash_table);

/*
 StringHashTableGet�֐�
 �n�b�V���e�[�u������l���擾����
 ����
 hash_table	: �n�b�V���e�[�u�����Ǘ�����f�[�^
 key		: ��������f�[�^�̃L�[
 �Ԃ�l
	�L�[�ɑΉ�����f�[�^ (������Ȃ��ꍇ��NULL)
*/
EXTERN void* StringHashTableGet(
	STRING_HASH_TABLE* hash_table,
	const char* key
);

/*
 StringHashTableAppend�֐�
 �n�b�V���e�[�u���ɒl��ǉ�����
 ����
 hash_table	: �n�b�V���e�[�u�����Ǘ�����f�[�^
 key		: �f�[�^�ɕt�^����L�[�̕�����
 data		: �ǉ�����f�[�^
 �Ԃ�l
	�ǉ������f�[�^��ID
*/
EXTERN int StringHashTableAppend(
	STRING_HASH_TABLE* hash_table,
	const char* key,
	void* data
);

/*
 StringHashTableRemove�֐�
 ��������L�[�Ƃ���n�b�V���e�[�u������1�f�[�^���폜����
 ����
 hash_table	: �n�b�V���e�[�u�����Ǘ�����f�[�^
 key		: �폜����f�[�^�̃L�[
*/
EXTERN void StringHashTableRemove(
	STRING_HASH_TABLE* hash_table,
	const char* key
);

/*
 FLAG_ON�}�N��
 �t���O�z��̑Ώۃt���O��ON�ɂ���
 ����
 flags	: �t���O�z��
 id		: ON�ɂ���t���O
*/
#define FLAG_ON(flags, id) ((flags)[((id) / (sizeof(*flags) * 8))] |= (1 << ((id) % (sizeof(flags) * 8))))

/*
 FLAG_OFF�}�N��
 �t���O�z��̑Ώۃt���O��OFF�ɂ���
 ����
 flags	: �t���O�z��
 id		: OFF�ɂ���t���O
*/
#define FLAG_OFF(flags, id) ((flags)[((id) / (sizeof(*flags) * 8))] &= ~(1 << ((id) % (sizeof(*flags) * 8))))

/*
 FLAG_CHECK�}�N��
 �t���O�z��̑Ώۃt���O��ON/OFF����
 ����
 flags	: �t���O�z��
 id		: ON/OFF���肷��t���O
 �Ԃ�l
 ON:0�ȊO	OFF:0
*/
#define FLAG_CHECK(flags, id) ((flags)[((id) / (sizeof(*flags) * 8))] & (1 << ((id) % (sizeof(*flags) * 8))))

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_UTILS_H_
