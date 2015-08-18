#ifndef _INCLUDED_MEMORY_POOL_H_
#define _INCLUDED_MEMORY_POOL_H_

#include <stdio.h>
#include "types.h"

/*
 MEMORY_POOL_BLOCK�\����
 �������v�[���̌ʃu���b�N
*/
typedef struct _MEMORY_POOL_BLOCK
{
	void *data;								// �f�[�^
	struct _MEMORY_POOL_BLOCK *next;		// ���̃u���b�N
} MEMORY_POOL_BLOCK;

/*
 MEMORY_POOL�\����
 �v���T�C�Y�ɉ������������A�h���X��Ԃ�
 (�ʂ̊J���͂ł��Ȃ�)
*/
typedef struct _MEMORY_POOL
{
	MEMORY_POOL_BLOCK *top;
	MEMORY_POOL_BLOCK *current;
	void *next_allocate;
	void *end_of_current;
	size_t block_size;
} MEMORY_POOL;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeMemoryPool�֐�
 �������v�[�����Ǘ�����f�[�^������������
 ����
 pool		: �������v�[�����Ǘ�����f�[�^
 block_size	: �������v�[���u���b�N1���̃T�C�Y
*/
EXTERN void InitializeMemoryPool(MEMORY_POOL* pool, size_t block_size);

/*
 ReleaseMemoryPool�֐�
 �������v�[���S�̂��J������
 ����
 pool	: �J�����郁�����v�[��
*/
EXTERN void ReleaseMemoryPool(MEMORY_POOL* pool);

/*
 MemoryPoolAllocate�֐�
 �������v�[���Ɏ��̃������m�ۂ�v������
 ����
 pool	: �������v�[�����Ǘ�����f�[�^
 size	: �v�����郁�����̃o�C�g��
 �Ԃ�l
	�m�ۂ����������A�h���X
*/
EXTERN void* MemoryPoolAllocate(MEMORY_POOL* pool, size_t size);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_MEMORY_POOL_H_
