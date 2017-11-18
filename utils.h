#ifndef _INCLUDED_UTILS_H_
#define _INCLUDED_UTILS_H_

#include <stdio.h>
#include <stdarg.h>
#include "types.h"

/*
 POINTER_ARRAY構造体
 可変長のポインタ型配列
*/
typedef struct _POINTER_ARRAY
{
	void **buffer;					// 配列の実体
	size_t buffer_size;				// 現在の配列のサイズ
	size_t num_data;				// 現在保持しているデータの数
	size_t block_size;				// 配列拡大時の更新幅
	void (*delete_func)(void*);		// データ削除時に使う関数ポインタ
} POINTER_ARRAY;

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
 ErrorMessage関数
 エラーメッセージを表示する
 引数
 format	: 表示するメッセージのフォーマット
*/
void ErrorMessage(const char* format, ... );

/*
 DummyFunctionNoReturn関数
 何もしないコールバック関数(返り値無し)
 引数
 dummy	: ダミーの引数
*/
extern void DummyFunctionNoReturn(void* dummy);

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
 DefaultErrorMessage関数
 デフォルトのエラー表示用関数
 引数
 message	: 表示するメッセージ
 dummy		: ダミーデータ
*/
extern void DefaultErrorMessage(const char* message, void* dummy);

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
EXTERN int BinarySearch(
	void* search_array,
	void* target,
	int block_size,
	int num_blocks,
	int(*compare_func)(void*, void*, void*),
	void* user_data
);

/*
 InitializePointerArray関数
 可変長のポインタ型配列を初期化する
 引数
 pointer_array	: 初期化する可変長ポインタ型配列
 block_size		: 配列のバッファサイズ拡大時の更新幅
 delete_func	: データ削除時に使う関数ポインタ
*/
EXTERN void InitializePointerArray(
	POINTER_ARRAY* pointer_array,
	size_t block_size,
	void (*delete_func)(void*)
);

/*
 ReleasePointerArray関数
 可変長ポインタ型配列を開放する
 引数
 pointer_array	: 可変長ポインタ型配列
*/
EXTERN void ReleasePointerArray(POINTER_ARRAY* pointer_array);

/*
 PointerArrayAppend関数
 可変長ポインタ型配列にデータを追加する
 引数
 pointer_array	: 可変長ポインタ型配列
 data			: 追加するデータ
*/
EXTERN void PointerArrayAppend(POINTER_ARRAY* pointer_array, void* data);

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
 PriorityArrayReset関数
 優先度付き配列の内容をリセットする
 引数
 priority_array	: 優先度付き配列
*/
EXTERN void PriorityArrayReset(PRIORITY_ARRAY* priority_array);

/*
 GetStringHashValue関数
 文字列のハッシュ値を取得する
 引数
 key	: ハッシュ値を取得する文字列
 返り値
	ハッシュ値
*/
EXTERN unsigned int GetStringHashValue(const char* key);

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

/*
 FLAG_ONマクロ
 フラグ配列の対象フラグをONにする
 引数
 flags	: フラグ配列
 id		: ONにするフラグ
*/
#define FLAG_ON(flags, id) ((flags)[((id) / (sizeof(*flags) * 8))] |= (1 << ((id) % (sizeof(flags) * 8))))

/*
 FLAG_OFFマクロ
 フラグ配列の対象フラグをOFFにする
 引数
 flags	: フラグ配列
 id		: OFFにするフラグ
*/
#define FLAG_OFF(flags, id) ((flags)[((id) / (sizeof(*flags) * 8))] &= ~(1 << ((id) % (sizeof(*flags) * 8))))

/*
 FLAG_CHECKマクロ
 フラグ配列の対象フラグのON/OFF判定
 引数
 flags	: フラグ配列
 id		: ON/OFF判定するフラグ
 返り値
 ON:0以外	OFF:0
*/
#define FLAG_CHECK(flags, id) ((flags)[((id) / (sizeof(*flags) * 8))] & (1 << ((id) % (sizeof(*flags) * 8))))

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_UTILS_H_
