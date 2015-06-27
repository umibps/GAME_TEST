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
 XOR_SHIFT32構造体
 テーブルサイズが32bitのXorShift擬似乱数生成器
*/
typedef struct _XOR_SHIFT32
{
	random_uint32 buffer;
} XOR_SHIFT32;

/*
 XOR_SHIFT64構造体
 テーブルサイズが64bitのXorShift擬似乱数生成器
*/
typedef struct _XOR_SHIFT64
{
	random_uint32 buffer[2];
} XOR_SHIFT64;

/*
 XOR_SHIFT96構造体
 テーブルサイズが96bitのXorShift擬似乱数生成器
*/
typedef struct _XOR_SHIFT96
{
	random_uint32 buffer[3];
} XOR_SHIFT96;

/*
 XOR_SHIFT128構造体
 テーブルサイズが128bitのXorShift擬似乱数生成器
*/
typedef struct _XOR_SHIFT128
{
	random_uint32 buffer[4];
} XOR_SHIFT128;

// RanrotBの定数群
#define RANROT_KK 17
#define RANROT_JJ 10
#define RANROT_R1 13
#define RANROT_R2 9
// RanrotBが生成する乱数の最大幅
#define RANROT_MAX ULONG_MAX
#define INTEGER_RANROT_MAX LONG_MAX

/*
 RANROT構造体
 RanrotB用のデータ
*/
typedef struct _RANROT
{
	int point1, point2;					// テーブル参照位置
	random_uint32 buffer[RANROT_KK];	// 乱数生成テーブル
	random_int32 minimum;				// 生成する乱数の最小値
	random_int32 maximum;				// 生成する乱数の最大値
	random_uint32 range;				// 生成する乱数の幅
} RANROT;

#ifdef __cplusplus
extern "C" {
#endif

/*
 SeedXorShift32関数
 32bitXorShift擬似乱数生成器の初期化
 引数
 table	: 乱数テーブル
 seed	: 乱数の種
*/
EXTERN void SeedXorShift32(XOR_SHIFT32* table, random_uint32 seed);

/*
 XorShift32GetNext関数
 32bitXorShiftの次の値を取得する
 引数
 table	: 乱数テーブル
 返り値
	0～0xFFFFFFFFの符号無し32bit整数の値
*/
EXTERN random_uint32 XorShift32GetNext(XOR_SHIFT32* table);

/*
 SeedXorShift64関数
 64bitXorShift擬似乱数生成器の初期化
 引数
 table	: 乱数テーブル
 seed	: 乱数の種
*/
EXTERN void SeedXorShift64(XOR_SHIFT64* table, random_uint32 seed);

/*
 XorShift64GetNext関数
 64bitXorShiftの次の値を取得する
 引数
 table	: 乱数テーブル
 返り値
	0～0xFFFFFFFFの符号無し32bit整数の値
*/
EXTERN random_uint32 XorShift64GetNext(XOR_SHIFT64* table);

/*
 SeedXorShift96関数
 96bitXorShift擬似乱数生成器の初期化
 引数
 table	: 乱数テーブル
 seed	: 乱数の種
*/
EXTERN void SeedXorShift96(XOR_SHIFT96* table, random_uint32 seed);

/*
 XorShift96GetNext関数
 96bitXorShiftの次の値を取得する
 引数
 table	: 乱数テーブル
 返り値
	0～0xFFFFFFFFの符号無し32bit整数の値
*/
EXTERN random_uint32 XorShift96GetNext(XOR_SHIFT96* table);

/*
 SeedXorShift128関数
 128bitXorShift擬似乱数生成器の初期化
 引数
 table	: 乱数テーブル
 seed	: 乱数の種
*/
EXTERN void SeedXorShift128(XOR_SHIFT128* table, random_uint32 seed);

/*
 XorShift128GetNext関数
 128bitXorShiftの次の値を取得する
 引数
 table	: 乱数テーブル
 返り値
	0～0xFFFFFFFFの符号無し32bit整数の値
*/
EXTERN random_uint32 XorShift128GetNext(XOR_SHIFT128* table);

/*
 RanrotGetNext関数
 RantotB擬似乱数の次の値を取得する
 引数
 table	: 乱数テーブル
 返り値
	0～0xFFFFFFFFの符号無し32bit整数
*/
EXTERN random_int32 RanrotGetNext(RANROT* table);

/*
 RanrotSetRange関数
 RantotBが生成する乱数の範囲を設定する
 引数
 table		: 乱数テーブル
 minimum	: 生成する乱数の最小値
 maximum	: 生成する乱数の最大値
*/
EXTERN void RanrotSetRange(RANROT* table, random_int32 minimum, random_int32 maximum);

/*
 SeedRanrot関数
 RanrotB擬似乱数生成器を初期化する
 引数
 table	: 乱数テーブル
 seed	: 乱数の種
*/
EXTERN void SeedRanrot(RANROT* table, random_uint32 seed);

/*
 RanrotGetNextInteger関数
 RanrotBの次の整数値を取得する
 引数
 talbe	: 乱数テーブル
 返り値
	指定された範囲(初期0～0x7FFFFFFF)の符号付き32bit整数値
*/
EXTERN random_int32 RanrotGetNextInteger(RANROT* table);

/*
 RanrotGetNextFloat関数
 RanrotBの次の0～1の倍精度浮動小数の値を取得する
 引数
 table	: 乱数テーブル
 返り値
	0～1の倍精度浮動小数の値
*/
EXTERN random_float RanrotGetNextFloat(RANROT* table);

/*
 ShuffleArray関数
 int型の配列をシャッフルする
 引数
 target			: シャッフルする配列(NULL指定可能)
 size			: 配列のサイズ
 start			: 開始位置(最初もランダムにしたい場合は-1)
 random_func	: 乱数生成に使う関数ポインタ
 random_table	: 乱数生成用のデータ
 返り値
	シャッフルされた配列
	(targetにNULLを指定した場合はMEM_ALLOC_FUNCした配列)
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
