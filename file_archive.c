#include <stdio.h>
#include <string.h>
#include "file_archive.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeFileArchive関数
 ファイルアーカイブ管理用データの初期化
 引数
 archive        : アーカイブを管理する構造体のアドレス
 archive_path   : アーカイブ・ファイルのパス
 stream         : アーカイブを読み込む為のデータ
 read_func      : データ読み込みに使う関数ポインタ
 seek_func      : データのシークに使う関数ポインタ
 tell_func      : データのシーク位置取得に使う関数ポインタ
 close_func     : アーカイブを閉じる際に使う関数ポインタ
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
int InitializeFileArchive(
	FILE_ARCHIVE* archive,
	const char* archive_path,
	void* stream,
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
    long (*tell_func)(void*),
    int (*close_func)(void*)
)
{
	uint8 data[8];
	uint32 data32;
	int i;

	(void)memset(archive, 0, sizeof(*archive));

	if(stream == NULL)
	{
		return FALSE;
	}

	archive->stream = stream;
	archive->read_func = read_func;
	archive->seek_func = seek_func;
	archive->tell_func = tell_func;
    archive->close_func = close_func;

	// ファイルの数を読み込む
	(void)read_func(data, 1, 4, stream);
	archive->num_files = (data[0] << 24) | (data[1] << 16)
		| (data[2] << 8) | data[3];

	// ヘッダ情報を読み込む領域を確保
	archive->files = (FILE_ARCHIVE_ITEM*)MEM_ALLOC_FUNC(sizeof(*archive->files)*archive->num_files);

	// ヘッダ情報を読み込む
	for(i=0; i<archive->num_files; i++)
	{
		// ファイル名のハッシュ値
		(void)read_func(data, 1, 4, stream);
		archive->files[i].hash_value = (data[0] << 24) | (data[1] << 16)
			| (data[2] << 8) | data[3];

		// ファイル名
		(void)read_func(data, 1, 4, stream);
		data32 = (data[0] << 24) | (data[1] << 16)
			| (data[2] << 8) | data[3];
		archive->files[i].file_name = (char*)MEM_ALLOC_FUNC(data32);
		(void)read_func(archive->files[i].file_name, 1, data32, stream);

		// ファイルの位置
		(void)read_func(data, 1, 4, stream);
		archive->files[i].data_position = (data[0] << 24) | (data[1] << 16)
			| (data[2] << 8) | data[3];

		// ファイルサイズ
		(void)read_func(data, 1, 4, stream);
		archive->files[i].data_size = (data[0] << 24) | (data[1] << 16)
			| (data[2] << 8) | data[3];
	}

	return TRUE;
}

/*
 ReleaseFileArchive関数
 ファイルアーカイブ管理用データの開放
 引数
 archive	: ファイルアーカイブ管理用データ
*/
void ReleaseFileArchive(FILE_ARCHIVE* archive)
{
	int i;

	for(i=0; i<archive->num_files; i++)
	{
		MEM_FREE_FUNC(archive->files[i].file_name);
	}
	MEM_FREE_FUNC(archive->files);

	archive->num_files = 0;

	if(archive->close_func != NULL)
	{
		(void)archive->close_func(archive->stream);
	}
}

static int CompareFileArchiveItem(const FILE_ARCHIVE_ITEM* item1, const FILE_ARCHIVE_ITEM* item2)
{
    if(item2->hash_value > item1->hash_value)
    {
        return -1;
    }
	return item1->hash_value - item2->hash_value;
}

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
FILE_ARCHIVE_READ* FileArchiveReadNew(const char* path, const char* mode, FILE_ARCHIVE* archive)
{
	FILE_ARCHIVE_READ *ret;
	FILE_ARCHIVE_ITEM item;
	int id;

	item.hash_value = GetStringHashValue(path);
	item.file_name = (char*)path;

	id = BinarySearch(archive->files, &item, sizeof(item),
		archive->num_files, (int(*)(void*, void*, void*))CompareFileArchiveItem, NULL);
	if(id < 0)
	{
		return NULL;
	}

	ret = (FILE_ARCHIVE_READ*)MEM_ALLOC_FUNC(sizeof(*ret));

	ret->item_data = archive->files[id];
	ret->current_position = 0;
    ret->archive = archive;

	return ret;
}

/*
 DeleteFileArchiveRead
 ファイルアーカイブ中のデータ読み込み用データを削除
 引数
 archive_item	: データ読み込み用のデータ
 返り値
	常に0
*/
int DeleteFileArchiveRead(FILE_ARCHIVE_READ* archive_item)
{
	MEM_FREE_FUNC(archive_item);

	return 0;
}

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
size_t FileArchiveRead(void* buffer, size_t block_size, size_t num_blocks, FILE_ARCHIVE_READ* archive_item)
{
	FILE_ARCHIVE *archive = archive_item->archive;
	size_t num_read;

	// 読み込み前にシークしておく
	(void)archive->seek_func(archive->stream,
		archive_item->item_data.data_position + archive_item->current_position, SEEK_SET);

	// 読み込むバイト数を計算
	num_read = (archive_item->current_position + block_size * num_blocks <= archive_item->item_data.data_size)
		? num_blocks : (archive_item->item_data.data_size - archive_item->current_position) / block_size;

	// データ読み込み位置を更新
	archive_item->current_position += (int)(num_read * block_size);

	return archive->read_func(buffer, block_size, num_read, archive->stream);
}

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
int FileArchiveSeek(FILE_ARCHIVE_READ* archive_item, long offset, int whence)
{
	int new_position = offset;

	switch(whence)
	{
	case SEEK_SET:
		break;
	case SEEK_CUR:
		new_position += archive_item->current_position;
		break;
	case SEEK_END:
		new_position = archive_item->item_data.data_size + offset;
		break;
	default:
		return -1;
	}

	if(new_position < 0)
	{
		new_position = 0;
	}
	else if(new_position > (int)archive_item->item_data.data_size)
	{
		new_position = archive_item->item_data.data_size;
	}

	archive_item->current_position = new_position;

	return 0;
}

/*
 FileArchiveTell関数
 ファイルアーカイブ読み込み位置を取得する
 引数
 archive_item	: データ読み込み用のデータ
 返り値
	読み込み位置
*/
long FileArchiveTell(FILE_ARCHIVE_READ* archive_item)
{
	return (long)archive_item->current_position;
}

#ifdef __cplusplus
}
#endif
