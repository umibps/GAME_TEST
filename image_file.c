// Visual Studio 2005以降では古いとされる関数を使用するので
// 警告が出ないようにする
#if defined _MSC_VER && _MSC_VER >= 1400
# define _CRT_SECURE_NO_DEPRECATE
# define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <stdio.h>
#include <setjmp.h>
#include <png.h>
#include "libjpeg/jpeglib.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 PNG_IO構造体
 libpngに読み書き関数ポインタを渡すためのデータ
 */
typedef struct _PNG_IO
{
	void *stream;
	size_t(*stream_func)(void*, size_t, size_t, void*);
	void(*flush_func)(void*);
} PNG_IO;

/*
 PngReadWrite関数
 PNGデータの圧縮・展開時の
 読み書き関数を呼び出すコールバック関数
 引数
 png_p	: libpngの圧縮・展開管理構造体
 data	: 読み込み格納先、書き込み元データのアドレス
 length	: 読み書きするバイト数
 */
static void PngReadWrite(
	png_structp png_p,
	png_bytep data,
	png_size_t length
	)
{
	PNG_IO *io = (PNG_IO*)png_get_io_ptr(png_p);
	(void)io->stream_func(data, 1, length, io->stream);
}

/*
 PngFlush関数
 ストリームに残ったデータをクリア
 引数
 png_p	: libpngの圧縮・展開管理構造体
 */
static void PngFlush(png_structp png_p)
{
	PNG_IO *io = (PNG_IO*)png_get_io_ptr(png_p);
	if(io->flush_func != NULL)
	{
		io->flush_func(io->stream);
	}
}

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
uint8* ReadPngStream(
	void* stream,
	size_t(*read_func)(void*, size_t, size_t, void*),
	int* width,
	int* height,
	int* channel
)
{
	PNG_IO io;
	png_structp png_p;
	png_infop info_p;
	png_uint_32 local_width, local_height;
	png_size_t local_stride;
	png_uint_32 bit_depth, color_type, interlace_type;
	uint8 *pixels = NULL;
	uint8 **pixel_array = NULL;
	int i;

	// データストリームのアドレスと関数ポインタをセット
	io.stream = stream;
	io.stream_func = read_func;

	// PNG展開用のデータを生成
	png_p = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	// 画像情報を格納するデータ領域作成
	info_p = png_create_info_struct(png_p);

	if (setjmp(png_jmpbuf(png_p)) != 0)
	{
		png_destroy_read_struct(&png_p, &info_p, NULL);
		MEM_FREE_FUNC(pixels);
		MEM_FREE_FUNC(pixel_array);
		return NULL;
	}

	// 読み込み用のストリーム、関数ポインタをセット
	png_set_read_fn(png_p, (void*)&io, (png_rw_ptr)PngReadWrite);

	// 画像の幅・高さ等の情報を読み込む
	png_read_info(png_p, info_p);
	png_get_IHDR(png_p, info_p, &local_width, &local_height,
		&bit_depth, &color_type, &interlace_type, NULL, NULL);
	local_stride = png_get_rowbytes(png_p, info_p);

	// ピクセル用のメモリを確保
	pixels = (uint8*)MEM_ALLOC_FUNC(local_stride*local_height);

	// libpngは2次元配列を渡す必要があるので擬似的に2次元配列作成
	pixel_array = (uint8**)MEM_ALLOC_FUNC(sizeof(*pixel_array)*local_height);
	for (i=0; i<(int)local_height; i++)
	{
		pixel_array[i] = &pixels[i*local_stride];
	}

	// ピクセルデータを展開しながら読み込み
	png_read_image(png_p, pixel_array);

	// メモリの開放
	png_destroy_read_struct(&png_p, &info_p, NULL);
	MEM_FREE_FUNC(pixel_array);

	// 幅・高さ・チャンネル数のデータをセット
	if(width != NULL)
	{
		*width = (int)local_width;
	}
	if(height != NULL)
	{
		*height = (int)local_height;
	}
	if(channel != NULL)
	{
		*channel = (int)(local_stride / local_width);
	}

	return pixels;
}

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
void WritePngStream(
	void* stream,
	size_t(*write_func)(void*, size_t, size_t, void*),
	void(*flush_func)(void*),
	uint8* pixels,
	int width,
	int height,
	int channel,
	int compression
)
{
	PNG_IO io ={stream, write_func, flush_func};	// 書き込み実行時のデータ
	png_structp png_p;								// PNG圧縮用のデータ
	png_infop info_p;								// 圧縮用データに画像情報を渡すデータ
	uint8 **pixel_arrays;							// 1次元→2次元配列用のポインタ配列
	int color_type;									// PNGの色のタイプ
	int stride = width * channel;					// 画像1行分のバイト数
	int i;

	// 画像のチャンネル数にあわせてカラータイプを設定
	switch(channel)
	{
	case 1:
		color_type = PNG_COLOR_TYPE_GRAY;
		break;
	case 2:
		color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
		break;
	case 3:
		color_type = PNG_COLOR_TYPE_RGB;
		break;
	case 4:
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		break;
	default:
		return;
	}

	// 書き込み用のデータを作成
	png_p = png_create_write_struct(
		PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	// 画像情報格納用のデータを作成
	info_p = png_create_info_struct(png_p);

	// 書き込み用のストリームと関数ポインタを設定
	png_set_write_fn(png_p, &io,
		(png_rw_ptr)PngReadWrite, (png_flush_ptr)PngFlush);
	// 圧縮には全てのフィルターを使用
	png_set_filter(png_p, 0, PNG_ALL_FILTERS);
	// 圧縮レベルを設定
	png_set_compression_level(png_p, compression);

	// PNGの情報をセット
	png_set_IHDR(png_p, info_p, width, height, 8, color_type,
		0, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// pnglib用に2次元配列を作成
	pixel_arrays = (uint8**)MEM_ALLOC_FUNC(height*sizeof(*pixel_arrays));
	for(i=0; i<height; i++)
	{
		pixel_arrays[i] = &pixels[stride*i];
	}

	// 画像データの書き込み
	png_write_info(png_p, info_p);
	png_write_image(png_p, pixel_arrays);
	png_write_end(png_p, info_p);

	// メモリの開放
	png_destroy_write_struct(&png_p, &info_p);

	MEM_FREE_FUNC(pixel_arrays);
}

/*
 JPEG_ERROR_MANAGER構造体
 JPEGデコード中のエラーを管理
*/
typedef struct _JPEG_ERROR_MANAGER
{
	struct jpeg_error_mgr jerr;	// libjpegから受けるエラーの情報
	jmp_buf buf;				// エラー時のジャンプ先
} JPEG_ERROR_MANAGER;

/*
 JpegErrorHandler関数
 JPEGデコード中のエラー処理
 引数
 cinfo	: JPEGの圧縮・展開用の共通情報
*/
static void JpegErrorHandler(j_common_ptr cinfo)
{
	JPEG_ERROR_MANAGER *manager = (JPEG_ERROR_MANAGER*)(cinfo->err);

	// 設定したジャンプ先へ移動
	longjmp(manager->buf, 1);
}

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
uint8* ReadJpegStream(
	void* stream,
	size_t(*read_func)(void*, size_t, size_t, void*),
	size_t data_size,
	int* width,
	int* height,
	int* channel
)
{
	// JPEGデコード用のデータ
	struct jpeg_decompress_struct decode;
	// デコードしたピクセルデータ
	uint8 *pixels = NULL;
	// データを一度メモリに全て読み込む
	uint8 *jpeg_data = (uint8*)MEM_ALLOC_FUNC(data_size);
	// ピクセルデータ擬似的に2次元配列にする
	uint8 **pixel_datas;
	// 画像の幅・高さ(ローカル)
	int local_width, local_height;
	// 画像のチャンネル数(ローカル)
	int local_channel;
	// 1行分のバイト数
	int stride;
	// デコード時のエラー処理
	JPEG_ERROR_MANAGER error;
	// for文用のカウンタ
	int i;

	if(jpeg_data == NULL)
	{
		return NULL;
	}

	// データの読み込み
	(void)read_func(jpeg_data, 1, data_size, stream);

	// エラー処理の設定
	error.jerr.error_exit = (noreturn_t (*)(j_common_ptr))JpegErrorHandler;
	decode.err = jpeg_std_error(&error.jerr);
	// エラー時のジャンプ先を設定
	if(setjmp(error.buf) != 0)
	{	// エラーで戻ってきた
		MEM_FREE_FUNC(jpeg_data);
		MEM_FREE_FUNC(pixel_datas);
		MEM_FREE_FUNC(pixels);
		return NULL;
	}

	// デコードデータの初期化
	jpeg_create_decompress(&decode);
	// メモリ上のJPEGデータをデコードするように設定
	jpeg_mem_src(&decode, jpeg_data, (unsigned long)data_size);
	
	// ヘッダの読み込み
	if(jpeg_read_header(&decode, TRUE) != JPEG_HEADER_OK)
	{
		return NULL;
	}

	// 画像の幅、高さ、チャンネル数を取得
	local_width = decode.image_width;
	local_height = decode.image_height;
	local_channel = decode.num_components;
	stride = local_channel * local_width;

	if(local_width != 0)
	{
		if(width != NULL)
		{
			*width = local_width;
		}
	}
	else
	{
		return NULL;
	}
	if(local_height != 0)
	{
		if(height != NULL)
		{
			*height = local_height;
		}
	}
	else
	{
		return NULL;
	}
	if(channel != NULL)
	{
		*channel = local_channel;
	}

	// ピクセルデータのメモリを確保
	pixels = (uint8*)MEM_ALLOC_FUNC(local_height * stride);
	pixel_datas = (uint8**)MEM_ALLOC_FUNC(sizeof(*pixel_datas) * local_height);
	for(i=0; i<local_height; i++)
	{
		pixel_datas[i] = &pixels[i*stride];
	}

	// デコード開始
	(void)jpeg_start_decompress(&decode);
	// データが残っている限りループ
	while(decode.output_scanline < decode.image_height)
	{
		jpeg_read_scanlines(&decode, pixel_datas + decode.output_scanline,
			decode.image_height - decode.output_scanline);
	}

	// メモリ開放
	jpeg_finish_decompress(&decode);
	MEM_FREE_FUNC(pixel_datas);
	jpeg_destroy_decompress(&decode);
	MEM_FREE_FUNC(jpeg_data);

	return pixels;
}

#ifdef __cplusplus
}
#endif
