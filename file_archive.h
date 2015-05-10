#ifndef _INCLUDED_FILE_ARCHIVE_H_
#define _INCLUDED_FILE_ARCHIVE_H_

#include "types.h"

/*
 FILE_ARCHIVE_ITEM構造体
 アーカイブ中の1ファイル
*/
typedef struct _FILE_ARCHIVE_ITEM
{
	uint32 hash_value;		// ファイル名のハッシュ値
	char *file_name;		// ファイル名
	uint32 data_position;	// ファイル先頭の位置
	uint32 data_size;		// ファイルサイズ
} FILE_ARCHIVE_ITEM;

/*
 FILE_ARCHIVE構造体
 アーカイブ1つ分のデータ
*/
typedef struct _FILE_ARCHIVE
{
	FILE_ARCHIVE_ITEM *files;	// ファイルの内容
	int num_files;				// ファイルの数
	void *stream;				// アーカイブデータへのポインタ
	// データ読み込み用の関数ポインタ
	size_t (*read_func)(void*, size_t, size_t, void*);
	// 読み込み位置移動用の関数ポインタ
	int (*seek_func)(void*, long, int);
	// 読み込み位置取得用の関数ポインタ
	long (*tell_func)(void*);
} FILE_ARCHIVE;

/*
 FILE_ARCHIVE_READ構造体
 アーカイブ中のファイル読み込み用のデータ
*/
typedef struct _FILE_ARCHIVE_READ
{
	FILE_ARCHIVE_ITEM item_data;	// ファイル名, ファイルサイズ等のデータ
	int current_position;			// 現在の読み込み位置
	FILE_ARCHIVE *archive;			// アーカイブ全体を管理するデータ
} FILE_ARCHIVE_READ;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeFileArchive関数
 ファイルアーカイブ管理用データの初期化
 引数
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
EXTERN int InitializeFileArchive(
	FILE_ARCHIVE* archive,
	const char* archive_path,
	void* stream,
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
	long (*tell_func)(void*)
);

/*
 ReleaseFileArchive関数
 ファイルアーカイブ管理用データの開放
 引数
 archive	: ファイルアーカイブ管理用データ
*/
EXTERN void ReleaseFileArchive(FILE_ARCHIVE* archive);

/*
 FileArchiveReadNew関数
 ファイルアーカイブ中のファイル読み込み用データ作成
 引数
 path		: ファイル名
 mode		: ダミー
 archive	: アーカイブ全体を管理するデータ
 返り値
	正常終了:データ読み込み用のデータ	異常終了:NULL
*/
EXTERN FILE_ARCHIVE_READ* FileArchiveReadNew(const char* path, const char* mode, FILE_ARCHIVE* archive);

/*
 DeleteFileArchiveRead
 ファイルアーカイブ中のデータ読み込み用データを削除
 引数
 archive_item	: データ読み込み用のデータ
 返り値
	常に0
*/
EXTERN int DeleteFileArchiveRead(FILE_ARCHIVE_READ* archive_item);

/*
 FileArchiveRead関数
 ファイルアーカイブ中のデータを読み込む
 引数
 buffer			: データを格納するバッファ
 block_size		: データ1つ分のバイト数
 num_blocks		: 読み込むデータ数
 archive_item	: データ読み込み用のデータ
 返り値
	読み込んだデータ数
*/
EXTERN size_t FileArchiveRead(void* buffer, size_t block_size, size_t num_blocks, FILE_ARCHIVE_READ* archive_item);

/*
 FileArchiveSeek関数
 ファイルアーカイブ読み込み位置を変更する
 引数
 archive_item	: データ読み込み用のデータ
 offset			: シーク開始位置からのオフセット
 whence			: シーク開始位置(fseekと同じ)
 返り値
	正常終了:0	異常終了:-1
*/
EXTERN int FileArchiveSeek(FILE_ARCHIVE_READ* archive_item, long offset, int whence);

/*
 FileArchiveTell関数
 ファイルアーカイブ読み込み位置を取得する
 引数
 archive_item	: データ読み込み用のデータ
 返り値
	読み込み位置
*/
EXTERN long FileArchiveTell(FILE_ARCHIVE_READ* archive_item);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_FILE_ARCHIVE_H_
