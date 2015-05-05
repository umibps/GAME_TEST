#ifndef _INCLUDED_SOUND_H_
#define _INCLUDED_SOUND_H_

#include <stdio.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>
#include <FLAC/stream_decoder.h>
#include "utils.h"
#include "types.h"

#define SOUND_BUFFER_SIZE 4096	// 音声バッファの1ブロックのバイト数
#define SOUND_BUFFER_NUM 8		// 音声バッファのブロック数

/*
 SOUND_CONTEXT構造体
 音声再生に必要なデータ
*/
typedef struct _SOUND_CONTEXT
{
	ALCdevice *device;		// 音声再生用のデバイス
	ALCcontext *context;	// 音声再生用のコンテキスト
} SOUND_CONTEXT;

typedef enum _eSOUND_PLAY_FLAGS
{
	SOUND_PLAY_NO_FLAG = 0x00,
	SOUND_PLAY_FLAG_END = 0x01,			// 再生終了
	SOUND_PLAY_FLAG_LOOP_PLAY = 0x02,	// ループ再生
	SOUND_PLAY_FLAG_PAUSE = 0x04		// 一時停止
} eSOUND_PLAY_FLAG;

/*
 SOUND_PLAY_BASE構造体
 音声再生用の基本データ
*/
typedef struct _SOUND_PLAY_BASE
{
	// コンテキスト
	SOUND_CONTEXT *context;
	// 音声データをデコードして取得する関数ポインタ
	size_t (*get_data_func)(uint8* data, size_t size, struct _SOUND_PLAY_BASE* sound_play);
	// 再生位置を変更する関数ポインタ
	int (*time_seek_func)(struct _SOUND_PLAY_BASE* sound_play, FLOAT_T time_value);
	// データ削除時に実行する関数ポインタ
	void (*delete_func)(void* stream);
	ALuint source;						// 音声ソース
	ALuint buffer[SOUND_BUFFER_NUM];	// 音声バッファ
	ALint play_state;					// 再生状態
	ALenum format;						// 音声のフォーマット
	ALsizei sample_rate;				// 音声のサンプリング・レート
	FLOAT_T loop_start_time;			// ループ再生の開始時間
	uint8 *sound_data;					// 音声データの一時保管場所
	int queue_size;						// 音声再生待ち行列のサイズ
	int next_unqueue;					// 次に待ち行列から外すバッファ
	unsigned int play_flags;			// 音声再生関連のフラグ
} SOUND_PLAY_BASE;

/*
 WAVE_SOUND_PLAY構造体
 無圧縮Waveサウンドを再生するためのデータ
*/
typedef struct _WAVE_SOUND_PLAY
{
	SOUND_PLAY_BASE base_data;	// 音声再生用の基本的なデータ
	int channel;				// チャンネル数(モノラル:1 ステレオ:2)
	int bit;					// 1サンプルあたりのビット数
	size_t data_size;			// データのサイズ
	int data_top;				// 音声データの先頭位置
	void *stream;				// 音声データストリーム
	// データ読み込みの関数ポインタ
	size_t (*read_func)(void*, size_t, size_t, void*);
	// データシーク用の関数ポインタ
	int (*seek_func)(void*, long, int);
	// データ削除時の関数ポインタ
	void (*delete_func)(void*);
} WAVE_SOUND_PLAY;

/*
 VORBIS_SOUND_PLAY構造体
 Vorbisサウンドを再生するためのデータ
*/
typedef struct _VORBIS_SOUND_PLAY
{
	SOUND_PLAY_BASE base_data;	// 音声再生用の基本的なデータ
	OggVorbis_File vorbis_file;	// デコードに必要なデータ
	vorbis_info *vorbis_info;	// 1サンプルあたりのビット数、サンプリングレート等のデータ
	size_t data_size;			// データのサイズ
	int current_position;		// 現在の再生位置
	void *stream;				// 音声データストリーム
	// データ読み込みの関数ポインタ
	size_t (*read_func)(void*, size_t, size_t, void*);
	// データシーク用の関数ポインタ
	int (*seek_func)(void*, long, int);
	// データシーク位置取得用の関数ポインタ
	long (*tell_func)(void*);
	// データ削除時の関数ポインタ
	void (*delete_func)(void*);
} VORBIS_SOUND_PLAY;

// バッファオーバーラン回避用一時保管場所のサイズ
#define FLAC_CARRY_BUFFER_SIZE (4096 * 4)

/*
 FLAC_SOUND_PLAY構造体
 FLACサウンドを再生するためのデータ
*/
typedef struct _FLAC_SOUND_PLAY
{
	SOUND_PLAY_BASE base_data;		// 音声再生用の基本的なデータ
	FLAC__StreamDecoder *decoder;	// デコードに必要なデータ
	size_t data_size;				// 音声データの総バイト数
	int write_position;				// 音声バッファに書き込んだバイト数
	int carry_size;					// 一時保管場所に貯めているバイト数
	int bits_per_sample;			// 1サンプルあたりのビット数
	int channels;					// ステレオ or モノラル
	// 一時保管用のバッファ
	uint8 carry_data[FLAC_CARRY_BUFFER_SIZE];
	void *stream;				// 音声データストリーム
	// データ読み込みの関数ポインタ
	size_t (*read_func)(void*, size_t, size_t, void*);
	// データシーク用の関数ポインタ
	int (*seek_func)(void*, long, int);
	// データシーク位置取得用の関数ポインタ
	long (*tell_func)(void*);
	// データ削除時の関数ポインタ
	void (*delete_func)(void*);
} FLAC_SOUND_PLAY;

// 音声データ配列のデフォルトのサイズ
#define SOUNDS_DEFAULT_BUFFER_SIZE 64

/*
 SOUND_ITEM構造体
 BGM, 効果音の再生アイテム
*/
typedef struct _SOUND_ITEM
{
	// データストリーム
	void *stream;
	// データ読み込み用関数ポインタ
	size_t (*read_func)(void*, size_t, size_t, void*);
	// データシーク用関数ポインタ
	int (*seek_func)(void*, long, int);
	// シーク位置取得用関数ポインタ
	long (*tell_func)(void*);
	// データ削除用の関数ポインタ
	void (*delete_func)(void*);
	// 音声再生用のデータ
	SOUND_PLAY_BASE *sound_play;
	// 音声全体を管理するデータ
	struct _SOUNDS *sounds;
} SOUND_ITEM;

/*
 SOUNDS構造体
 BGM, 効果音を管理
*/
typedef struct _SOUNDS
{
	SOUND_ITEM **sounds;	// 音声データ配列
	size_t num_sounds;		// 現在、保持している音声の数
	size_t buffer_size;		// 音声データ配列のサイズ
	size_t search_start;	// 検索開始位置
	SOUND_CONTEXT *context;	// 音声再生用の基本データ
} SOUNDS;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeSoundContext関数
 音声再生に必要な基本的なデータを初期化
 引数
 context	: 音声再生に必要な基本的なデータ
*/
EXTERN void InitializeSoundContext(SOUND_CONTEXT* context);

/*
 ReleaseSoundContext関数
 音声再生に必要な基本的なデータを開放
 引数
 context	: 音声再生に必要な基本的なデータ
*/
EXTERN void ReleaseSoundContext(SOUND_CONTEXT* context);

/*
 InitializeSoundPlayBase関数
 音声再生用の基本データを初期化
 引数
 sound_play		: 音声再生用の基本的なデータ
 context		: 音声再生用のコンテキスト
 get_data_func	: 音声データをデコードして取得する関数ポインタ
 time_seek_func	: 時間で再生位置をシークするための関数ポインタ
 delete_func	: データ削除時に使う関数ポインタ
 format			: 音声データのフォーマット
 sample_rate	: 音声データのサンプリングレート
 play_flags		: 音声再生のフラグ
*/
EXTERN void InitializeSoundPlayBase(
	SOUND_PLAY_BASE* sound_play,
	SOUND_CONTEXT* context,
	size_t(*get_data_func)(uint8* data, size_t size, SOUND_PLAY_BASE* sound_play),
	int(*time_seek_func)(SOUND_PLAY_BASE* sound_play, FLOAT_T time_value),
	void(*delete_func)(void* stream),
	ALenum format,
	ALsizei sample_rate,
	unsigned int play_flags
);

/*
 ReleaseSoundPlay関数
 音声再生用の基本的なデータを開放する
 引数
 sound_play	: 音声再生用の基本的なデータ
*/
EXTERN void ReleaseSoundPlay(SOUND_PLAY_BASE* sound_play);

/*
 PlaySound関数
 音声再生を実行する
 引数
 sound_play	: 音声再生用の基本的なデータ
*/
EXTERN void PlaySound(SOUND_PLAY_BASE* sound_play);

/*
 PauseSound関数
 音声再生を一時停止する
 引数
 sound_play	: 音声再生用の基本的なデータ
*/
EXTERN void PauseSound(SOUND_PLAY_BASE* sound);

/*
 StartPlaySound関数
 音声再生を開始する
 sound_play	: 音声再生用の基本的なデータ
*/
EXTERN void StartPlaySound(SOUND_PLAY_BASE* sound);

/*
 InitializerWaveSoundPlay関数
 無圧縮Waveサウンドデータを再生するためのデータを初期化
 引数
 sound_play		: 無圧縮Waveサウンドデータを再生するためのデータ
 context		: 音声再生用のコンテキスト
 stream			: 音声データストリーム
 read_func		: データ読み込み用の関数ポインタ
 seek_func		: データシーク用の関数ポインタ
 tell_func		: ストリームの位置取得用の関数ポインタ
 delete_func	: データ削除時に使う関数ポインタ
 play_flags		: 再生関連のフラグ
 返り値
	正常終了:TRUE	失敗:FALSE
*/
EXTERN int InitializeWaveSoundPlay(
	WAVE_SOUND_PLAY* sound_play,
	SOUND_CONTEXT* context,
	void* stream,
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	void (*delete_func)(void*),
	unsigned int play_flags
);

/*
 InitializerVorbisSoundPlay関数
 Vorbisサウンドデータを再生するためのデータを初期化
 引数
 sound_play		: Vorbisサウンドデータを再生するためのデータ
 context		: 音声再生用のコンテキスト
 stream			: 音声データストリーム
 read_func		: データ読み込み用の関数ポインタ
 seek_func		: データシーク用の関数ポインタ
 tell_func		: ストリームの位置取得用の関数ポインタ
 delete_func	: データ削除時に使う関数ポインタ
 play_flags		: 再生関連のフラグ
 返り値
	正常終了:TRUE	失敗:FALSE
*/
EXTERN int InitializeVorbisSoundPlay(
	VORBIS_SOUND_PLAY* sound_play,
	SOUND_CONTEXT* context,
	void* stream,
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	void (*delete_func)(void*),
	unsigned int play_flags
);

/*
 InitializerFlacSoundPlay関数
 FLACサウンドデータを再生するためのデータを初期化
 引数
 sound_play		: FLACサウンドデータを再生するためのデータ
 context		: 音声再生用のコンテキスト
 stream			: 音声データストリーム
 read_func		: データ読み込み用の関数ポインタ
 seek_func		: データシーク用の関数ポインタ
 tell_func		: ストリームの位置取得用の関数ポインタ
 delete_func	: データ削除時に使う関数ポインタ
 play_flags		: 再生関連のフラグ
 返り値
	正常終了:TRUE	失敗:FALSE
*/
EXTERN int InitializeFlacSoundPlay(
	FLAC_SOUND_PLAY* sound_play,
	SOUND_CONTEXT* context,
	void* stream,
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	void (*delete_func)(void*),
	unsigned int play_flags
);

/*
 InitiaizeSounds関数
 BGM, 効果音全体を管理する構造体を初期化
 引数
 sounds		: BGM, 効果音全体を管理する構造体
 context	: 音声再生用の基本データ
*/
EXTERN void InitializeSounds(SOUNDS* sounds, SOUND_CONTEXT* context);

/*
 AddSound関数
 BGM, または効果音を追加する
 引数
 sounds	: BGM, 効果音全体を管理する構造体
 open_func		: データを開くための関数ポインタ
 read_func		: データ読み込み用の関数ポインタ
 seek_func		: シーク位置を変更するための関数ポインタ
 tell_func		: シーク位置を取得するための関数ポインタ
 delete_func	: データを閉じるための関数ポインタ
 play_flags		: 音声再生のフラグ
 user_data		: データを開く際に使うユーザー定義データ
 返り値
	音声再生を管理するデータ
*/
EXTERN SOUND_PLAY_BASE* AddSound(
	SOUNDS* sounds,
	const char* path,
	void* (*open_func)(const char*, const char*, void*),
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	void (*delete_func)(void*),
	unsigned int play_flags,
	void* user_data
);

/*
 PlayAllSound関数
 全てのBGM, 効果音を再生する
 引数
 sounds	: BGM, 効果音全体を管理する構造体
*/
EXTERN void PlayAllSound(SOUNDS* sounds);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SOUND_H_
