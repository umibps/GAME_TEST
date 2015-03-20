#ifndef _INCLUDED_IMAGE_FILE_H_
#define _INCLUDED_IMAGE_FILE_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 ReadPngStream関数
 PNGイメージデータを読み込みデコードする
 引数
 stream		: データストリーム
 read_func	: 読み込みに使用する関数ポインタ
 width		: 画像の幅を受ける変数のアドレス
 height		: 画像の高さを受ける変数のアドレス
 channel	: 画像のチャンネル数を受ける変数のアドレス
 返り値
	成功:ピクセルデータ	失敗:NULL
*/
EXTERN uint8* ReadPngStream(
	void* stream,
	size_t(*read_func)(void*, size_t, size_t, void*),
	int* width,
	int* height,
	int* channel
);

/*
 WritePngStream関数
 PNGイメージデータをエンコードして書き出す
 引数
 stream			: データストリーム
 write_func		: 読み込みに使用する関数ポインタ
 flush_func		: バッファクリア用の関数ポインタ(NULL指定可能)
 pixels			: 画像のピクセルデータ
 width			: 画像の幅
 height			: 画像の高さ
 channel		: 画像のチャンネル数
 compression	: 圧縮レベル
*/
EXTERN void WritePngStream(
	void* stream,
	size_t(*write_func)(void*, size_t, size_t, void*),
	void(*flush_func)(void*),
	uint8* pixels,
	int width,
	int height,
	int channel,
	int compression
);

/*
 ReadJpegStream関数
 引数
 stream		: データストリーム
 read_func	: 読み込みに使用する関数ポインタ
 data_size	: 画像データのバイト数
 width		: 画像の幅を受ける変数のアドレス
 height		: 画像の高さを受ける変数のアドレス
 channel	: 画像のチャンネル数を受ける変数のアドレス
 返り値
	成功:ピクセルデータ	失敗:NULL
*/
EXTERN uint8* ReadJpegStream(
	void* stream,
	size_t(*read_func)(void*, size_t, size_t, void*),
	size_t data_size,
	int* width,
	int* height,
	int* channel
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_IMAGE_FILE_H_
