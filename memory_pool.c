#include <string.h>
#include <stdlib.h>
#include "memory_pool.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 MemoryPoolBlockAllocate�֐�
 �������v�[���u���b�N��V�����쐬����
 ����
 size	: �u���b�N�̃T�C�Y
 �Ԃ�l
	�쐬�����������v�[���u���b�N
*/
static MEMORY_POOL_BLOCK* MemoryPoolBlockAllocate(size_t size)
{
	MEMORY_POOL_BLOCK *ret = (MEMORY_POOL_BLOCK*)MEM_ALLOC_FUNC(
		sizeof(*ret) + size);
	ret->data = (void*)((uint8*)ret + sizeof(*ret));
	ret->next = NULL;
	return ret;
}

/*
 InitializeMemoryPool�֐�
 �������v�[�����Ǘ�����f�[�^������������
 ����
 pool		: �������v�[�����Ǘ�����f�[�^
 block_size	: �������v�[���u���b�N1���̃T�C�Y
*/
void InitializeMemoryPool(MEMORY_POOL* pool, size_t block_size)
{
	(void)memset(pool, 0, sizeof(*pool));

	pool->block_size = block_size;
	pool->top = pool->current = MemoryPoolBlockAllocate(pool->block_size);
	pool->next_allocate = pool->current->data;
	pool->end_of_current = (void*)((uint8*)pool->next_allocate + pool->block_size);
}

/*
 ReleaseMemoryPool�֐�
 �������v�[���S�̂��J������
 ����
 pool	: �J�����郁�����v�[��
*/
void ReleaseMemoryPool(MEMORY_POOL* pool)
{
	MEMORY_POOL_BLOCK *next_free = pool->top;
	MEMORY_POOL_BLOCK *target = pool->top;

	while(next_free != NULL)
	{
		target = next_free;
		next_free = next_free->next;
		MEM_FREE_FUNC(target);
	}
}

/*
 MemoryPoolAllocate�֐�
 �������v�[���Ɏ��̃������m�ۂ�v������
 ����
 pool	: �������v�[�����Ǘ�����f�[�^
 size	: �v�����郁�����̃o�C�g��
 �Ԃ�l
	�m�ۂ����������A�h���X
*/
void* MemoryPoolAllocate(MEMORY_POOL* pool, size_t size)
{
	void *ret = NULL;
	void *next_allocate = (void*)((uint8*)pool->next_allocate + size);

	if(next_allocate < pool->end_of_current)
	{	// ���̃u���b�N�쐬�͕s�v
		ret = pool->next_allocate;
		pool->next_allocate = next_allocate;
	}
	else
	{	// ���̃u���b�N�쐬���K�v
		MEMORY_POOL_BLOCK *new_block;
		if(size > pool->block_size)
		{	// �v���T�C�Y���ݒ�u���b�N�T�C�Y���傫��
				// �u���b�N��2�쐬
			new_block = MemoryPoolBlockAllocate(size);
			ret = new_block->data;
			pool->current->next = new_block;
			pool->current = new_block;
			new_block = MemoryPoolBlockAllocate(pool->block_size);
			pool->current->next = new_block;
			pool->current = new_block;
			pool->next_allocate = new_block->data;
			pool->end_of_current = (void*)((uint8*)pool->next_allocate + pool->block_size);
		}
		else
		{	// �v���T�C�Y���ݒ�u���b�N�T�C�Y�ȉ�
				// �u���b�N��1�쐬
			new_block = MemoryPoolBlockAllocate(pool->block_size);
			ret = new_block->data;
			pool->current->next = new_block;
			pool->current = new_block;
			pool->next_allocate = (void*)((uint8*)ret + size);
			pool->end_of_current = (void*)((uint8*)new_block->data + pool->block_size);
		}
	}

	return ret;
}

#ifdef __cplusplus
}
#endif
