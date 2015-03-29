#ifndef _INCLUDED_UTILS_H_
#define _INCLUDED_UTILS_H_

#include <stdio.h>
#include "types.h"

/*
 PRIORITY_ARRAY_ITEM構造体
 優先度付き配列のひとつの項目
*/
typedef struct _PRIORITY_ARRAY_ITEM
{
	void *data;				// 汎用ポインタ
	unsigned int priority;	// 優先度
} PRIORITY_ARRAY_ITEM;

/*
 PRIORITY_ARRAY構造体
 優先度付き配列
*/
typedef struct _PRIORITY_ARRAY
{
	PRIORITY_ARRAY_ITEM *buffer;	// 待ち行列の実体
	size_t buffer_size;				// 現在の待ち行列のサイズ
	size_t num_data;				// 現在保持している項目の数
	size_t block_size;				// 待ち行列のサイズ拡大時の更新幅
	void (*delete_func)(void*);		// データ削除時に使う関数ポインタ
} PRIORITY_ARRAY;

/*
 HASH_TABLE_ITEM構造体
 文字列をキーとするハッシュテーブルの1つの項目のデータ
*/
typedef struct _STRING_HASH_TABLE_ITEM
{
	void *data;					// 汎用ポインタ
	unsigned int hash_value;	// ハッシュ値
	const char *key;			// キーの文字列
} STRING_HASH_TABLE_ITEM;

/*
 STRING_HASH_TABLE構造体
 文字列をキーとするハッシュテーブル
*/
typedef struct _STRING_HASH_TABLE
{
	STRING_HASH_TABLE_ITEM *buffer;	// ハッシュテーブルの実体
	unsigned int buffer_size;		// 現在のハッシュテーブルのサイズ
	unsigned int num_data;			// 現在保持している項目の数
	unsigned int block_size;		// ハッシュテーブルのサイズ拡大時の更新幅
	void (*delete_func)(void*);		// データ削除時に使う関数ポインタ
} STRING_HASH_TABLE;

/*
 RGBAマクロ関数
 赤緑青α値から32ビット符号無し整数を作成
*/
#define RGBA(R, G, B, A) (((R) << 24) | ((G) << 16) | ((B) << 8) | (A))

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
EXTERN int StringCompareIgnoreCase(const char* str1, const char* str2);

/*
 GetNextUtf8Character関数
 次のUTF8の文字を取得する
 引数
 str	: 次の文字を取得したい文字列
 返り値
	次の文字のアドレス
*/
EXTERN const char* GetNextUtf8Character(const char* str);

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
EXTERN FILE* FileOpen(const char* path, const char* mode, void* user_data);

/*
 FileClose関数
 ファイルを閉じる
 引数
 fp			: ファイルポインタ
 user_data	: プログラマーが自由に使えるデータ
 返り値
	常に0
*/
EXTERN int FileClose(FILE* fp, void* user_data);

/*
 InitializePriorityArray関数
 優先度付き配列を初期化する
 引数
 priority_array	: 初期化する優先度付き配列
 block_size		: バッファサイズ拡大時の更新幅
 delete_func	: データ削除時に使う関数ポインタ
*/
EXTERN void InitializePriorityArray(
	PRIORITY_ARRAY* priority_array,
	size_t block_size,
	void (*delete_func)(void*)
);

/*
 ReleasePriorityArray関数
 優先度付き配列を開放する
 引数
 priority_array	: 優先度付き配列
*/
EXTERN void ReleasePriorityArray(PRIORITY_ARRAY* priority_array);

/*
 PriorityArrayAppend関数
 優先度付き配列にデータを1つ追加する
 引数
 priority_array	: 優先度付き配列
 data			: 追加するデータのアドレス
 priority		: 追加するデータの優先度
*/
EXTERN void PriorityArrayAppend(
	PRIORITY_ARRAY* priority_array,
	void* data,
	unsigned int priority
);

/*
 PriorityArrayRemove関数
 優先度付き配列からデータを1つ取り除く
 引数
 priority_array	: 優先度付き配列
 data			: 取り除くデータ
*/
EXTERN void PriorityArrayRemove(
	PRIORITY_ARRAY* priority_array,
	void* data
);

/*
 InitializeStringHashTable関数
 文字列をキーとするハッシュテーブルを初期化する
 引数
 hash_table		: ハッシュテーブルを管理するデータ
 block_size		: バッファサイズを更新する際の更新幅
 delete_func	: データ削除時に使う関数ポインタ
*/
EXTERN void InitializeStringHashTable(
	STRING_HASH_TABLE* hash_table,
	size_t block_size,
	void (*delete_func)(void*)
);

/*
 ReleaseStringHashTable関数
 文字列をキーとするハッシュテーブルを開放する
 引数
 hash_table	: ハッシュテーブルを管理するデータ
*/
EXTERN void ReleaseStringHashTable(STRING_HASH_TABLE* hash_table);

/*
 StringHashTableGet関数
 ハッシュテーブルから値を取得する
 引数
 hash_table	: ハッシュテーブルを管理するデータ
 key		: 検索するデータのキー
 返り値
	キーに対応するデータ (見つからない場合はNULL)
*/
EXTERN void* StringHashTableGet(
	STRING_HASH_TABLE* hash_table,
	const char* key
);

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
EXTERN int StringHashTableAppend(
	STRING_HASH_TABLE* hash_table,
	const char* key,
	void* data
);

/*
 StringHashTableRemove関数
 文字列をキーとするハッシュテーブルから1つデータを削除する
 引数
 hash_table	: ハッシュテーブルを管理するデータ
 key		: 削除するデータのキー
*/
EXTERN void StringHashTableRemove(
	STRING_HASH_TABLE* hash_table,
	const char* key
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_UTILS_H_
