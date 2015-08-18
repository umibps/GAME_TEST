#ifndef _INCLUDED_MEMORY_POOL_H_
#define _INCLUDED_MEMORY_POOL_H_

#include <stdio.h>
#include "types.h"

/*
 MEMORY_POOL_BLOCK構造体
 メモリプールの個別ブロック
*/
typedef struct _MEMORY_POOL_BLOCK
{
	void *data;								// データ
	struct _MEMORY_POOL_BLOCK *next;		// 次のブロック
} MEMORY_POOL_BLOCK;

/*
 MEMORY_POOL構造体
 要求サイズに応じたメモリアドレスを返す
 (個別の開放はできない)
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
 InitializeMemoryPool関数
 メモリプールを管理するデータを初期化する
 引数
 pool		: メモリプールを管理するデータ
 block_size	: メモリプールブロック1つ分のサイズ
*/
EXTERN void InitializeMemoryPool(MEMORY_POOL* pool, size_t block_size);

/*
 ReleaseMemoryPool関数
 メモリプール全体を開放する
 引数
 pool	: 開放するメモリプール
*/
EXTERN void ReleaseMemoryPool(MEMORY_POOL* pool);

/*
 MemoryPoolAllocate関数
 メモリプールに次のメモリ確保を要求する
 引数
 pool	: メモリプールを管理するデータ
 size	: 要求するメモリのバイト数
 返り値
	確保したメモリアドレス
*/
EXTERN void* MemoryPoolAllocate(MEMORY_POOL* pool, size_t size);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_MEMORY_POOL_H_
