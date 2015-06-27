#include <stdio.h>
#include <ctype.h>
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 StringCompareIgnoreCase関数
 大文字/小文字を無視して文字列を比較
 引数
 str1	: 比較する文字列その1
 str2	: 比較する文字列その2
 返り値
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
 GetNextUtf8Character関数
 次のUTF8の文字を取得する
 引数
 str	: 次の文字を取得したい文字列
 返り値
	次の文字のアドレス
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
 FileOpen関数
 ファイルを開く
 引数
 path		: ファイルのパス
 mode		: 開くモード(fopen関数に準拠)
 user_data	: プログラマーが自由に使えるデータ
 返り値
	成功:ファイルポインタ	失敗:NULL
*/
FILE* FileOpen(const char* path, const char* mode, void* user_data)
{
	return fopen(path, mode);
}

/*
 FileClose関数
 ファイルを閉じる
 引数
 fp			: ファイルポインタ
 user_data	: プログラマーが自由に使えるデータ
 返り値
	常に0
*/
int FileClose(FILE* fp, void* user_data)
{
	(void)fclose(fp);

	return 0;
}

/*
 BinarySearch関数
 二分探索を実行する
 引数
 search_array	: 探索される配列
 target			: 探索を行うデータ
 block_size		: 配列の1つ分の要素のバイト数
 num_blocks		: 配列のサイズ
 compare_func	: データ比較用の関数
					(同値:0 大なり:1 小なり:-1を返す)
 user_data		: compare_funcの第3引数に入るユーザーデータ
 返り値
	発見した要素のID (見つからない場合は最も近い左の負数)
*/
int BinarySearch(
	void* search_array,
	void* target,
	int block_size,
	int num_blocks,
	int(*compare_func)(void*, void*, void*),
	void* user_data
)
{
	unsigned char *bytes = (unsigned char*)search_array;
	int compare;
	int minimum = 0;
	int maximum = num_blocks - 1;
	int midium;

	while(minimum <= maximum)
	{
		midium = (minimum + maximum) / 2;
		compare = compare_func((void*)&bytes[midium*block_size], target, user_data);
		if(compare == 0)
		{
			while(midium < num_blocks-1
				&& compare_func(&bytes[(midium+1)*block_size], target, user_data) == 0)
			{
				midium++;
			}
			return midium;
		}
		else if(compare < 0)
		{
			minimum = midium + 1;
		}
		else
		{
			maximum = midium - 1;
		}
	}

	if(maximum < 0)
	{
		maximum = 0;
	}
	while(maximum > 0
		&& compare_func(&bytes[(maximum-1)*block_size],
		&bytes[maximum*block_size], user_data) > 0)
	{
		maximum--;
	}
	while(maximum < num_blocks
		&& compare_func(&bytes[(maximum+1)*block_size],
		&bytes[maximum*block_size], user_data) < 0)
	{
		maximum++;
	}

	return - maximum;
}

/*
 InitializePriorityArray関数
 優先度付き配列を初期化する
 引数
 priority_array	: 初期化する優先度付き配列
 block_size		: バッファサイズ拡大時の更新幅
 delete_func	: データ削除時に使う関数ポインタ
*/
void InitializePriorityArray(
	PRIORITY_ARRAY* priority_array,
	size_t block_size,
	void (*delete_func)(void*)
)
{
	(void)memset(priority_array, 0, sizeof(priority_array));

	priority_array->block_size = priority_array->buffer_size
		= block_size;
	priority_array->buffer =
		(PRIORITY_ARRAY_ITEM*)MEM_CALLOC_FUNC(block_size, sizeof(*priority_array->buffer));
	priority_array->delete_func = delete_func;
}

/*
 ReleasePriorityArray関数
 優先度付き配列を開放する
 引数
 priority_array	: 優先度付き配列
*/
void ReleasePriorityArray(PRIORITY_ARRAY* priority_array)
{
	if(priority_array->delete_func != NULL)
	{
		unsigned int i;
		for(i=0; i<(unsigned int)priority_array->num_data; i++)
		{
			priority_array->delete_func(priority_array->buffer[i].data);
		}
	}
	MEM_FREE_FUNC(priority_array->buffer);
}

/*
 ComparePriorityArrayItem関数
 二分探索用の比較関数
 引数
 item1	: 比較対象1
 item2	: 比較対象2
 dummy	: 汎用ポインタ(使用しない)
 返り値
	優先度高い:-1	優先度同等:0	優先度低い:1以上
*/
static int ComparePriorityArrayItem(
	PRIORITY_ARRAY_ITEM* item1,
	PRIORITY_ARRAY_ITEM* item2,
	void* dummy
)
{
	if(item1->priority < item2->priority)
	{
		return -1;
	}

	return item1->priority - item2->priority;
}

/*
 PriorityArrayAppend関数
 優先度付き配列にデータを1つ追加する
 引数
 priority_array	: 優先度付き配列
 data			: 追加するデータのアドレス
 priority		: 追加するデータの優先度
*/
void PriorityArrayAppend(
	PRIORITY_ARRAY* priority_array,
	void* data,
	unsigned int priority
)
{
	PRIORITY_ARRAY_ITEM item = {data, priority};

	if(priority_array->num_data == 0)
	{
		priority_array->buffer[0] = item;
	}
	else
	{
		int place = BinarySearch(priority_array->buffer, &item,
			sizeof(item), priority_array->num_data,
			(int(*)(void*, void*, void*))ComparePriorityArrayItem, NULL);
		unsigned int i;
		if(place >= 0)
		{
			place++;
		}
		else
		{
			place = -place;
		}
		for(i=priority_array->num_data; i>(unsigned int)place; i--)
		{
			priority_array->buffer[i] = priority_array->buffer[i-1];
		}
		priority_array->buffer[place] = item;
	}
	priority_array->num_data++;

	if(priority_array->num_data == priority_array->buffer_size)
	{
		size_t before_size = priority_array->buffer_size;
		priority_array->buffer_size += priority_array->block_size;
		priority_array->buffer = (PRIORITY_ARRAY_ITEM*)MEM_REALLOC_FUNC(
			priority_array->buffer, sizeof(*priority_array->buffer)*priority_array->buffer_size);
		(void)memset(&priority_array->buffer[before_size], 0,
			sizeof(*priority_array->buffer)*priority_array->block_size);
	}
}

/*
 PriorityArrayRemove関数
 優先度付き配列からデータを1つ取り除く
 引数
 priority_array	: 優先度付き配列
 data			: 取り除くデータ
*/
void PriorityArrayRemove(
	PRIORITY_ARRAY* priority_array,
	void* data
)
{
	unsigned int i;
	for(i=0; i<(unsigned int)priority_array->num_data; i++)
	{
		if(priority_array->buffer[i].data == data)
		{
			unsigned int j;
			for(j=priority_array->num_data-1; j>i; j++)
			{
				priority_array->buffer[i-1] = priority_array->buffer[i];
			}
			if(priority_array->delete_func != NULL)
			{
				priority_array->delete_func(data);
			}
			priority_array->num_data--;
			return;
		}
	}
}

/*
 PriorityArrayReset関数
 優先度付き配列の内容をリセットする
 引数
 priority_array	: 優先度付き配列
*/
void PriorityArrayReset(PRIORITY_ARRAY* priority_array)
{
	unsigned int i;

	if(priority_array->delete_func != NULL)
	{
		for(i=0; i<(unsigned int)priority_array->num_data; i++)
		{
			priority_array->delete_func(priority_array->buffer[i].data);
		}
	}

	(void)memset(priority_array->buffer, 0,
		sizeof(*priority_array->buffer)*priority_array->buffer_size);

	priority_array->num_data = 0;
}

/*
 InitializeStringHashTable関数
 文字列をキーとするハッシュテーブルを初期化する
 引数
 hash_table		: ハッシュテーブルを管理するデータ
 block_size		: バッファサイズを更新する際の更新幅
 delete_func	: データ削除時に使う関数ポインタ
*/
void InitializeStringHashTable(
	STRING_HASH_TABLE* hash_table,
	size_t block_size,
	void (*delete_func)(void*)
)
{
	(void)memset(hash_table, 0, sizeof(hash_table));

	hash_table->block_size = block_size;
	hash_table->buffer =
		(STRING_HASH_TABLE_ITEM*)MEM_CALLOC_FUNC(block_size, sizeof(*hash_table->buffer));
	hash_table->delete_func = delete_func;
}

/*
 ReleaseStringHashTable関数
 文字列をキーとするハッシュテーブルを開放する
 引数
 hash_table	: ハッシュテーブルを管理するデータ
*/
void ReleaseStringHashTable(STRING_HASH_TABLE* hash_table)
{
	unsigned int i;

	if(hash_table->delete_func == NULL)
	{
		for(i=0; i<hash_table->num_data; i++)
		{
			MEM_FREE_FUNC((void*)hash_table->buffer[i].key);
		}
	}
	else
	{
		for(i=0; i<hash_table->num_data; i++)
		{
			hash_table->delete_func(hash_table->buffer[i].data);
			MEM_FREE_FUNC((void*)hash_table->buffer[i].key);
		}
	}
}

/*
 CompareStringHashTableItem関数
 二分探索用の比較関数
 引数
 item1	: 比較対象1
 item2	: 比較対象2
 dummy	: 汎用ポインタ(使用しない)
 返り値
	ハッシュ値小さい:-1	ハッシュ値同値:0	ハッシュ値大きい:1
*/
static int CompareStringHashTableItem(
	STRING_HASH_TABLE_ITEM* item1,
	STRING_HASH_TABLE_ITEM* item2,
	void* dummy
)
{
	if(item1->hash_value == item2->hash_value)
	{
		return strcmp(item1->key, item2->key);
	}
	else if(item1->hash_value < item2->hash_value)
	{
		return -1;
	}

	return 1;
}

/*
 GetStringHashValue関数
 文字列のハッシュ値を取得する
 引数
 key	: ハッシュ値を取得する文字列
 返り値
	ハッシュ値
*/
unsigned int GetStringHashValue(const char* key)
{
#define INITIAL_FNV (2166136261u)
#define FNV_MULTIPLE (16777619u)
	unsigned int hash_value = INITIAL_FNV;
	const char *str = key;

	while(*str != '\0')
	{
		hash_value = hash_value ^ (*str);
		hash_value *= FNV_MULTIPLE;
		str++;
	}

	return hash_value;
}

/*
 StringHashTableGet関数
 ハッシュテーブルから値を取得する
 引数
 hash_table	: ハッシュテーブルを管理するデータ
 key		: 検索するデータのキー
 返り値
	キーに対応するデータ (見つからない場合はNULL)
*/
void* StringHashTableGet(
	STRING_HASH_TABLE* hash_table,
	const char* key
)
{
	STRING_HASH_TABLE_ITEM item;
	int place;

	item.hash_value = GetStringHashValue(key);
	item.key = key;

	if(hash_table->num_data == 0)
	{
		return NULL;
	}

	place = BinarySearch(hash_table->buffer, &item,
		sizeof(item), (int)hash_table->num_data,
		(int(*)(void*, void*, void*))CompareStringHashTableItem, NULL
	);

	if(place < 0)
	{
		return NULL;
	}
	else if(place == 0 && item.hash_value != hash_table->buffer[place].hash_value)
	{
		return NULL;
	}

	return hash_table->buffer[place].data;
}

/*
 StringHashTableAppend関数
 ハッシュテーブルに値を追加する
 引数
 hash_table	: ハッシュテーブルを管理するデータ
 key		: データに付与するキーの文字列
 data		: 追加するデータ
 返り値
	追加したデータのID
*/
int StringHashTableAppend(
	STRING_HASH_TABLE* hash_table,
	const char* key,
	void* data
)
{
	STRING_HASH_TABLE_ITEM item;
	int place;

	item.hash_value = GetStringHashValue(key);
	item.key = key;
	item.data = data;

	if(hash_table->num_data == 0)
	{
		item.key = MEM_STRDUP_FUNC(key);
		hash_table->buffer[0] = item;
		hash_table->num_data++;
		return 0;
	}
	place = BinarySearch(hash_table->buffer, &item,
		sizeof(item), (int)hash_table->num_data,
		(int (*)(void*, void*, void*))CompareStringHashTableItem, NULL
	);

	if(place < 0)
	{
		unsigned int i;

		place = - place;

		for(i=hash_table->num_data; i>(unsigned int)place; i--)
		{
			hash_table->buffer[i] = hash_table->buffer[i-1];
		}
		hash_table->buffer[place] = item;

		hash_table->num_data++;

		if(hash_table->num_data == hash_table->buffer_size)
		{
			size_t before_size = hash_table->buffer_size;
			hash_table->buffer_size += hash_table->block_size;
			hash_table->buffer = (STRING_HASH_TABLE_ITEM*)MEM_REALLOC_FUNC(
				hash_table->buffer, sizeof(*hash_table->buffer)*hash_table->buffer_size);
			(void)memset(&hash_table->buffer[before_size], 0,
				sizeof(*hash_table->buffer)*hash_table->block_size);
		}

		return place;
	}

	return place;
}

/*
 StringHashTableRemove関数
 文字列をキーとするハッシュテーブルから1つデータを削除する
 引数
 hash_table	: ハッシュテーブルを管理するデータ
 key		: 削除するデータのキー
*/
void StringHashTableRemove(
	STRING_HASH_TABLE* hash_table,
	const char* key
)
{
	STRING_HASH_TABLE_ITEM item;
	int place;

	item.hash_value = GetStringHashValue(key);
	item.key = key;

	if(hash_table->num_data == 0)
	{
		return;
	}

	place = BinarySearch(hash_table->buffer, &item,
		sizeof(item), (int)hash_table->num_data,
		(int (*)(void*, void*, void*))CompareStringHashTableItem, NULL
	);
	if(place >= 0)
	{
		unsigned int i;

		item.data = hash_table->buffer[place].data;
		MEM_FREE_FUNC((void*)hash_table->buffer[place].key);

		for(i=hash_table->num_data-1; i>=(unsigned int)place; i++)
		{
			hash_table->buffer[i] = hash_table->buffer[i-1];
		}
		hash_table->num_data--;

		if(hash_table->delete_func != NULL)
		{
			hash_table->delete_func(item.data);
		}
	}
}

#ifdef __cplusplus
}
#endif
