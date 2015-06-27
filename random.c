#include "random.h"

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
void SeedXorShift32(XOR_SHIFT32* table, random_uint32 seed)
{
	table->buffer = (seed == 0) ? 2463534242UL : seed;
}

/*
 XorShift32GetNext関数
 32bitXorShiftの次の値を取得する
 引数
 table	: 乱数テーブル
 返り値
	0～0xFFFFFFFFの符号無し32bit整数の値
*/
random_uint32 XorShift32GetNext(XOR_SHIFT32* table)
{
	random_uint32 x = table->buffer;
	x ^= (x << 13);
	x ^= (x >> 17);
	table->buffer = (x ^= (x << 5));
	return x;
}

/*
 SeedXorShift64関数
 64bitXorShift擬似乱数生成器の初期化
 引数
 table	: 乱数テーブル
 seed	: 乱数の種
*/
void SeedXorShift64(XOR_SHIFT64* table, random_uint32 seed)
{
	table->buffer[0] = seed = 1812433253UL * (seed ^ (seed >> 30)) + 1;
	table->buffer[1] = 1812433253UL * (seed ^ (seed >> 30)) + 2;
}

/*
 XorShift64GetNext関数
 64bitXorShiftの次の値を取得する
 引数
 table	: 乱数テーブル
 返り値
	0～0xFFFFFFFFの符号無し32bit整数の値
*/
random_uint32 XorShift64GetNext(XOR_SHIFT64* table)
{
#define A 10
#define B 13
#define C 10
	random_uint32 t, y;
	random_uint32 *a = table->buffer;
	t = a[0];
	t = (t ^ (t << A));
	a[0] = y = a[1];
	t = (y ^ (y >> C)) ^ (t ^ (t >> B));
	a[1] = t;

	return t;
#undef A
#undef B
#undef C
}

/*
 SeedXorShift96関数
 96bitXorShift擬似乱数生成器の初期化
 引数
 table	: 乱数テーブル
 seed	: 乱数の種
*/
void SeedXorShift96(XOR_SHIFT96* table, random_uint32 seed)
{
	int i;
	for(i=0; i<3; i++)
	{
		table->buffer[i] = seed = 1812433253UL * (seed ^ (seed >> 30)) + i+1;
	}
}

/*
 XorShift96GetNext関数
 96bitXorShiftの次の値を取得する
 引数
 table	: 乱数テーブル
 返り値
	0～0xFFFFFFFFの符号無し32bit整数の値
*/
random_uint32 XorShift96GetNext(XOR_SHIFT96* table)
{
#define A 10
#define B 1
#define C 26
	random_uint32 t, z;
	random_uint32 *a = table->buffer;
	t = a[0];
	t = (t ^ (t << A));
	a[0] = a[1];
	a[1] = z = a[2];
	t = (z ^ (z >> C)) ^ (t ^ (t >> B));
	a[2] = t;

	return t;
#undef A
#undef B
#undef C
}

/*
 SeedXorShift128関数
 128bitXorShift擬似乱数生成器の初期化
 引数
 table	: 乱数テーブル
 seed	: 乱数の種
*/
void SeedXorShift128(XOR_SHIFT128* table, random_uint32 seed)
{
	int i;
	for(i=0; i<4; i++)
	{
		table->buffer[i] = seed = 1812433253UL * (seed ^ (seed >> 30)) + i+1;
	}
}

/*
 XorShift128GetNext関数
 128bitXorShiftの次の値を取得する
 引数
 table	: 乱数テーブル
 返り値
	0～0xFFFFFFFFの符号無し32bit整数の値
*/
random_uint32 XorShift128GetNext(XOR_SHIFT128* table)
{
#define A 11
#define B 8
#define C 19
	random_uint32 t, w;
	random_uint32 *a = table->buffer;
	t = a[0];
	a[0] = a[1];
	a[1] = a[2];
	a[2] = w = a[3];
	t = t ^ (t << A);
	w = (w ^ (w >> C)) ^ (t ^ (t >> B));
	a[3] = w;

	return w;
#undef A
#undef B
#undef C
}

#define ROTATE(X, R) (((X) << (R) | ((X) >> (sizeof((X))*8-(R)))))

/*
 RanrotGetNext関数
 RantotB擬似乱数の次の値を取得する
 引数
 table	: 乱数テーブル
 返り値
	0～0xFFFFFFFFの符号無し32bit整数
*/
random_int32 RanrotGetNext(RANROT* table)
{
	random_uint32 result;

	// 乱数生成
	result = table->buffer[table->point1] = ROTATE(table->buffer[table->point2], RANROT_R1)
		+ ROTATE(table->buffer[table->point1], RANROT_R2);

	// 参照位置を更新
	table->point1--;
	if(table->point1 < 0)
	{
		table->point1 = 0;
	}
	table->point2--;
	if(table->point2 < 0)
	{
		table->point2 = 0;
	}

	return result;
}

/*
 RanrotSetRange関数
 RantotBが生成する乱数の範囲を設定する
 引数
 table		: 乱数テーブル
 minimum	: 生成する乱数の最小値
 maximum	: 生成する乱数の最大値
*/
void RanrotSetRange(RANROT* table, random_int32 minimum, random_int32 maximum)
{
	if(minimum > maximum)
	{
		table->maximum = minimum;
		table->minimum = maximum;
	}
	else
	{
		table->minimum = minimum;
		table->maximum = maximum;
	}
	table->range = (random_uint32)(table->maximum - table->minimum);
}

/*
 SeedRanrot関数
 RanrotB擬似乱数生成器を初期化する
 引数
 table	: 乱数テーブル
 seed	: 乱数の種
*/
void SeedRanrot(RANROT* table, random_uint32 seed)
{
	int i;

	RanrotSetRange(table, 0, INTEGER_RANROT_MAX);

	if(seed == 0)
	{
		seed--;
	}

	for(i=0; i<RANROT_KK; i++)
	{
		seed ^= seed << 13;
		seed ^= seed >> 17;
		seed ^= seed << 5;
		table->buffer[i] = seed;
	}

	table->point1 = 0;
	table->point2 = RANROT_JJ;

	// 初期状態の偏りを回避
	for(i=0; i<10; i++)
	{
		(void)RanrotGetNext(table);
	}
}

/*
 RanrotGetNextInteger関数
 RanrotBの次の整数値を取得する
 引数
 talbe	: 乱数テーブル
 返り値
	指定された範囲(初期0～0x7FFFFFFF)の符号付き32bit整数値
*/
random_int32 RanrotGetNextInteger(RANROT* table)
{
	if(table->range <= 0)
	{
		return 0;
	}

	return (RanrotGetNext(table) % table->range) + table->minimum;
}

/*
 RanrotGetNextFloat関数
 RanrotBの次の0～1の倍精度浮動小数の値を取得する
 引数
 table	: 乱数テーブル
 返り値
	0～1の倍精度浮動小数の値
*/
random_float RanrotGetNextFloat(RANROT* table)
{
	return RanrotGetNext(table) * (1.0/4294967295.0);
}

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
int* ShuffleArray(
	int* target,
	int size,
	int start,
	random_int32 (*random_func)(void*),
	void* random_table
)
{
	int *rest;				// まだ取り出していない要素
	int remain = size - 1;	// まだ取り出していない要素の数
	int next;				// 次に取り出す要素のインデックス
	int i, j;

	// 配列のサイズチェック
	if(size <= 0)
	{
		return target;
	}

	if(target == NULL)
	{
		target = (int*)MEM_ALLOC_FUNC(sizeof(*target)*size);
	}

	// シャッフル用の一時保存領域を確保
	rest = (int*)MEM_ALLOC_FUNC(sizeof(*rest)*size);
	for(i=0; i<size; i++)
	{
		rest[i] = i;
	}

	if(start < 0 || start >= size)
	{	// 開始位置が指定されていないのでランダムに選定
		target[0] = (int)((unsigned int)random_func(random_table) % (unsigned int)size);
	}
	else
	{	// 開始位置を設定
		target[0] = start;
	}
	// 最初の要素をまだ取り出していない要素配列から削除
	for(i=target[0]; i<remain-1; i++)
	{
		rest[i] = rest[i+1];
	}

	// 残りの要素を選定
	for(i=1; remain > 0; i++, remain--)
	{	// 次の値を決定
		next = (int)((unsigned int)random_func(random_table) % (unsigned int)remain);
		target[i] = rest[next];
		// 取り出した要素をまだ取り出していない要素配列から削除
		for(j=next; j<remain-1; j++)
		{
			rest[j] = rest[j+1];
		}
	}

	MEM_FREE_FUNC(rest);

	return target;
}

#ifdef __cplusplus
}
#endif
