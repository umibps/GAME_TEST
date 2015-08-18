#include <string.h>
#include <stdlib.h>
#include "memory_pool.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 MemoryPoolBlockAllocate関数
 メモリプールブロックを新しく作成する
 引数
 size	: ブロックのサイズ
 返り値
	作成したメモリプールブロック
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
 InitializeMemoryPool関数
 メモリプールを管理するデータを初期化する
 引数
 pool		: メモリプールを管理するデータ
 block_size	: メモリプールブロック1つ分のサイズ
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
 ReleaseMemoryPool関数
 メモリプール全体を開放する
 引数
 pool	: 開放するメモリプール
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
 MemoryPoolAllocate関数
 メモリプールに次のメモリ確保を要求する
 引数
 pool	: メモリプールを管理するデータ
 size	: 要求するメモリのバイト数
 返り値
	確保したメモリアドレス
*/
void* MemoryPoolAllocate(MEMORY_POOL* pool, size_t size)
{
	void *ret = NULL;
	void *next_allocate = (void*)((uint8*)pool->next_allocate + size);

	if(next_allocate < pool->end_of_current)
	{	// 次のブロック作成は不要
		ret = pool->next_allocate;
		pool->next_allocate = next_allocate;
	}
	else
	{	// 次のブロック作成が必要
		MEMORY_POOL_BLOCK *new_block;
		if(size > pool->block_size)
		{	// 要求サイズが設定ブロックサイズより大きい
				// ブロックを2個作成
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
		{	// 要求サイズが設定ブロックサイズ以下
				// ブロックを1個作成
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
