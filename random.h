#ifndef _INCLUDED_RANDOM_H_
#define _INCLUDED_RANDOM_H_

#include <limits.h>
#include "types.h"

#if defined(_MSC_VER)

typedef long random_int32;
typedef unsigned long random_uint32;

#else

#include <stdint.h>

typedef int32_t random_int32;
typedef uint32_t random_uint32;

#endif

typedef double random_float;

/*
 XOR_SHIFT32�\����
 �e�[�u���T�C�Y��32bit��XorShift�[������������
*/
typedef struct _XOR_SHIFT32
{
	random_uint32 buffer;
} XOR_SHIFT32;

/*
 XOR_SHIFT64�\����
 �e�[�u���T�C�Y��64bit��XorShift�[������������
*/
typedef struct _XOR_SHIFT64
{
	random_uint32 buffer[2];
} XOR_SHIFT64;

/*
 XOR_SHIFT96�\����
 �e�[�u���T�C�Y��96bit��XorShift�[������������
*/
typedef struct _XOR_SHIFT96
{
	random_uint32 buffer[3];
} XOR_SHIFT96;

/*
 XOR_SHIFT128�\����
 �e�[�u���T�C�Y��128bit��XorShift�[������������
*/
typedef struct _XOR_SHIFT128
{
	random_uint32 buffer[4];
} XOR_SHIFT128;

// RanrotB�̒萔�Q
#define RANROT_KK 17
#define RANROT_JJ 10
#define RANROT_R1 13
#define RANROT_R2 9
// RanrotB���������闐���̍ő啝
#define RANROT_MAX ULONG_MAX
#define INTEGER_RANROT_MAX LONG_MAX

/*
 RANROT�\����
 RanrotB�p�̃f�[�^
*/
typedef struct _RANROT
{
	int point1, point2;					// �e�[�u���Q�ƈʒu
	random_uint32 buffer[RANROT_KK];	// ���������e�[�u��
	random_int32 minimum;				// �������闐���̍ŏ��l
	random_int32 maximum;				// �������闐���̍ő�l
	random_uint32 range;				// �������闐���̕�
} RANROT;

#ifdef __cplusplus
extern "C" {
#endif

/*
 SeedXorShift32�֐�
 32bitXorShift�[������������̏�����
 ����
 table	: �����e�[�u��
 seed	: �����̎�
*/
EXTERN void SeedXorShift32(XOR_SHIFT32* table, random_uint32 seed);

/*
 XorShift32GetNext�֐�
 32bitXorShift�̎��̒l���擾����
 ����
 table	: �����e�[�u��
 �Ԃ�l
	0�`0xFFFFFFFF�̕�������32bit�����̒l
*/
EXTERN random_uint32 XorShift32GetNext(XOR_SHIFT32* table);

/*
 SeedXorShift64�֐�
 64bitXorShift�[������������̏�����
 ����
 table	: �����e�[�u��
 seed	: �����̎�
*/
EXTERN void SeedXorShift64(XOR_SHIFT64* table, random_uint32 seed);

/*
 XorShift64GetNext�֐�
 64bitXorShift�̎��̒l���擾����
 ����
 table	: �����e�[�u��
 �Ԃ�l
	0�`0xFFFFFFFF�̕�������32bit�����̒l
*/
EXTERN random_uint32 XorShift64GetNext(XOR_SHIFT64* table);

/*
 SeedXorShift96�֐�
 96bitXorShift�[������������̏�����
 ����
 table	: �����e�[�u��
 seed	: �����̎�
*/
EXTERN void SeedXorShift96(XOR_SHIFT96* table, random_uint32 seed);

/*
 XorShift96GetNext�֐�
 96bitXorShift�̎��̒l���擾����
 ����
 table	: �����e�[�u��
 �Ԃ�l
	0�`0xFFFFFFFF�̕�������32bit�����̒l
*/
EXTERN random_uint32 XorShift96GetNext(XOR_SHIFT96* table);

/*
 SeedXorShift128�֐�
 128bitXorShift�[������������̏�����
 ����
 table	: �����e�[�u��
 seed	: �����̎�
*/
EXTERN void SeedXorShift128(XOR_SHIFT128* table, random_uint32 seed);

/*
 XorShift128GetNext�֐�
 128bitXorShift�̎��̒l���擾����
 ����
 table	: �����e�[�u��
 �Ԃ�l
	0�`0xFFFFFFFF�̕�������32bit�����̒l
*/
EXTERN random_uint32 XorShift128GetNext(XOR_SHIFT128* table);

/*
 RanrotGetNext�֐�
 RantotB�[�������̎��̒l���擾����
 ����
 table	: �����e�[�u��
 �Ԃ�l
	0�`0xFFFFFFFF�̕�������32bit����
*/
EXTERN random_int32 RanrotGetNext(RANROT* table);

/*
 RanrotSetRange�֐�
 RantotB���������闐���͈̔͂�ݒ肷��
 ����
 table		: �����e�[�u��
 minimum	: �������闐���̍ŏ��l
 maximum	: �������闐���̍ő�l
*/
EXTERN void RanrotSetRange(RANROT* table, random_int32 minimum, random_int32 maximum);

/*
 SeedRanrot�֐�
 RanrotB�[�����������������������
 ����
 table	: �����e�[�u��
 seed	: �����̎�
*/
EXTERN void SeedRanrot(RANROT* table, random_uint32 seed);

/*
 RanrotGetNextInteger�֐�
 RanrotB�̎��̐����l���擾����
 ����
 talbe	: �����e�[�u��
 �Ԃ�l
	�w�肳�ꂽ�͈�(����0�`0x7FFFFFFF)�̕����t��32bit�����l
*/
EXTERN random_int32 RanrotGetNextInteger(RANROT* table);

/*
 RanrotGetNextFloat�֐�
 RanrotB�̎���0�`1�̔{���x���������̒l���擾����
 ����
 table	: �����e�[�u��
 �Ԃ�l
	0�`1�̔{���x���������̒l
*/
EXTERN random_float RanrotGetNextFloat(RANROT* table);

/*
 ShuffleArray�֐�
 int�^�̔z����V���b�t������
 ����
 target			: �V���b�t������z��(NULL�w��\)
 size			: �z��̃T�C�Y
 start			: �J�n�ʒu(�ŏ��������_���ɂ������ꍇ��-1)
 random_func	: ���������Ɏg���֐��|�C���^
 random_table	: ���������p�̃f�[�^
 �Ԃ�l
	�V���b�t�����ꂽ�z��
	(target��NULL���w�肵���ꍇ��MEM_ALLOC_FUNC�����z��)
*/
EXTERN int* ShuffleArray(
	int* target,
	int size,
	int start,
	random_int32 (*random_func)(void*),
	void* random_table
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_RANDOM_H_
