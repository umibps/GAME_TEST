// Visual Studio 2005�ȍ~�ł͌Â��Ƃ����֐����g�p����̂�
// �x�����o�Ȃ��悤�ɂ���
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
 PNG_IO�\����
 libpng�ɓǂݏ����֐��|�C���^��n�����߂̃f�[�^
 */
typedef struct _PNG_IO
{
	void *stream;
	size_t(*stream_func)(void*, size_t, size_t, void*);
	void(*flush_func)(void*);
} PNG_IO;

/*
 PngReadWrite�֐�
 PNG�f�[�^�̈��k�E�W�J����
 �ǂݏ����֐����Ăяo���R�[���o�b�N�֐�
 ����
 png_p	: libpng�̈��k�E�W�J�Ǘ��\����
 data	: �ǂݍ��݊i�[��A�������݌��f�[�^�̃A�h���X
 length	: �ǂݏ�������o�C�g��
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
 PngFlush�֐�
 �X�g���[���Ɏc�����f�[�^���N���A
 ����
 png_p	: libpng�̈��k�E�W�J�Ǘ��\����
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
 ReadPngStream�֐�
 PNG�C���[�W�f�[�^��ǂݍ��݃f�R�[�h����
 ����
 stream		: �f�[�^�X�g���[��
 read_func	: �ǂݍ��݂Ɏg�p����֐��|�C���^
 width		: �摜�̕����󂯂�ϐ��̃A�h���X
 height		: �摜�̍������󂯂�ϐ��̃A�h���X
 channel	: �摜�̃`�����l�������󂯂�ϐ��̃A�h���X
 �Ԃ�l
	����:�s�N�Z���f�[�^	���s:NULL
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

	// �f�[�^�X�g���[���̃A�h���X�Ɗ֐��|�C���^���Z�b�g
	io.stream = stream;
	io.stream_func = read_func;

	// PNG�W�J�p�̃f�[�^�𐶐�
	png_p = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	// �摜�����i�[����f�[�^�̈�쐬
	info_p = png_create_info_struct(png_p);

	if (setjmp(png_jmpbuf(png_p)) != 0)
	{
		png_destroy_read_struct(&png_p, &info_p, NULL);
		MEM_FREE_FUNC(pixels);
		MEM_FREE_FUNC(pixel_array);
		return NULL;
	}

	// �ǂݍ��ݗp�̃X�g���[���A�֐��|�C���^���Z�b�g
	png_set_read_fn(png_p, (void*)&io, (png_rw_ptr)PngReadWrite);

	// �摜�̕��E�������̏���ǂݍ���
	png_read_info(png_p, info_p);
	png_get_IHDR(png_p, info_p, &local_width, &local_height,
		&bit_depth, &color_type, &interlace_type, NULL, NULL);
	local_stride = png_get_rowbytes(png_p, info_p);

	// �s�N�Z���p�̃��������m��
	pixels = (uint8*)MEM_ALLOC_FUNC(local_stride*local_height);

	// libpng��2�����z���n���K�v������̂ŋ[���I��2�����z��쐬
	pixel_array = (uint8**)MEM_ALLOC_FUNC(sizeof(*pixel_array)*local_height);
	for (i=0; i<(int)local_height; i++)
	{
		pixel_array[i] = &pixels[i*local_stride];
	}

	// �s�N�Z���f�[�^��W�J���Ȃ���ǂݍ���
	png_read_image(png_p, pixel_array);

	// �������̊J��
	png_destroy_read_struct(&png_p, &info_p, NULL);
	MEM_FREE_FUNC(pixel_array);

	// ���E�����E�`�����l�����̃f�[�^���Z�b�g
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
 WritePngStream�֐�
 PNG�C���[�W�f�[�^���G���R�[�h���ď����o��
 ����
 stream			: �f�[�^�X�g���[��
 write_func		: �ǂݍ��݂Ɏg�p����֐��|�C���^
 flush_func		: �o�b�t�@�N���A�p�̊֐��|�C���^(NULL�w��\)
 pixels			: �摜�̃s�N�Z���f�[�^
 width			: �摜�̕�
 height			: �摜�̍���
 channel		: �摜�̃`�����l����
 compression	: ���k���x��
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
	PNG_IO io ={stream, write_func, flush_func};	// �������ݎ��s���̃f�[�^
	png_structp png_p;								// PNG���k�p�̃f�[�^
	png_infop info_p;								// ���k�p�f�[�^�ɉ摜����n���f�[�^
	uint8 **pixel_arrays;							// 1������2�����z��p�̃|�C���^�z��
	int color_type;									// PNG�̐F�̃^�C�v
	int stride = width * channel;					// �摜1�s���̃o�C�g��
	int i;

	// �摜�̃`�����l�����ɂ��킹�ăJ���[�^�C�v��ݒ�
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

	// �������ݗp�̃f�[�^���쐬
	png_p = png_create_write_struct(
		PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	// �摜���i�[�p�̃f�[�^���쐬
	info_p = png_create_info_struct(png_p);

	// �������ݗp�̃X�g���[���Ɗ֐��|�C���^��ݒ�
	png_set_write_fn(png_p, &io,
		(png_rw_ptr)PngReadWrite, (png_flush_ptr)PngFlush);
	// ���k�ɂ͑S�Ẵt�B���^�[���g�p
	png_set_filter(png_p, 0, PNG_ALL_FILTERS);
	// ���k���x����ݒ�
	png_set_compression_level(png_p, compression);

	// PNG�̏����Z�b�g
	png_set_IHDR(png_p, info_p, width, height, 8, color_type,
		0, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// pnglib�p��2�����z����쐬
	pixel_arrays = (uint8**)MEM_ALLOC_FUNC(height*sizeof(*pixel_arrays));
	for(i=0; i<height; i++)
	{
		pixel_arrays[i] = &pixels[stride*i];
	}

	// �摜�f�[�^�̏�������
	png_write_info(png_p, info_p);
	png_write_image(png_p, pixel_arrays);
	png_write_end(png_p, info_p);

	// �������̊J��
	png_destroy_write_struct(&png_p, &info_p);

	MEM_FREE_FUNC(pixel_arrays);
}

/*
 JPEG_ERROR_MANAGER�\����
 JPEG�f�R�[�h���̃G���[���Ǘ�
*/
typedef struct _JPEG_ERROR_MANAGER
{
	struct jpeg_error_mgr jerr;	// libjpeg����󂯂�G���[�̏��
	jmp_buf buf;				// �G���[���̃W�����v��
} JPEG_ERROR_MANAGER;

/*
 JpegErrorHandler�֐�
 JPEG�f�R�[�h���̃G���[����
 ����
 cinfo	: JPEG�̈��k�E�W�J�p�̋��ʏ��
*/
static void JpegErrorHandler(j_common_ptr cinfo)
{
	JPEG_ERROR_MANAGER *manager = (JPEG_ERROR_MANAGER*)(cinfo->err);

	// �ݒ肵���W�����v��ֈړ�
	longjmp(manager->buf, 1);
}

/*
 ReadJpegStream�֐�
 ����
 stream		: �f�[�^�X�g���[��
 read_func	: �ǂݍ��݂Ɏg�p����֐��|�C���^
 data_size	: �摜�f�[�^�̃o�C�g��
 width		: �摜�̕����󂯂�ϐ��̃A�h���X
 height		: �摜�̍������󂯂�ϐ��̃A�h���X
 channel	: �摜�̃`�����l�������󂯂�ϐ��̃A�h���X
 �Ԃ�l
	����:�s�N�Z���f�[�^	���s:NULL
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
	// JPEG�f�R�[�h�p�̃f�[�^
	struct jpeg_decompress_struct decode;
	// �f�R�[�h�����s�N�Z���f�[�^
	uint8 *pixels = NULL;
	// �f�[�^����x�������ɑS�ēǂݍ���
	uint8 *jpeg_data = (uint8*)MEM_ALLOC_FUNC(data_size);
	// �s�N�Z���f�[�^�[���I��2�����z��ɂ���
	uint8 **pixel_datas;
	// �摜�̕��E����(���[�J��)
	int local_width, local_height;
	// �摜�̃`�����l����(���[�J��)
	int local_channel;
	// 1�s���̃o�C�g��
	int stride;
	// �f�R�[�h���̃G���[����
	JPEG_ERROR_MANAGER error;
	// for���p�̃J�E���^
	int i;

	if(jpeg_data == NULL)
	{
		return NULL;
	}

	// �f�[�^�̓ǂݍ���
	(void)read_func(jpeg_data, 1, data_size, stream);

	// �G���[�����̐ݒ�
	error.jerr.error_exit = (noreturn_t (*)(j_common_ptr))JpegErrorHandler;
	decode.err = jpeg_std_error(&error.jerr);
	// �G���[���̃W�����v���ݒ�
	if(setjmp(error.buf) != 0)
	{	// �G���[�Ŗ߂��Ă���
		MEM_FREE_FUNC(jpeg_data);
		MEM_FREE_FUNC(pixel_datas);
		MEM_FREE_FUNC(pixels);
		return NULL;
	}

	// �f�R�[�h�f�[�^�̏�����
	jpeg_create_decompress(&decode);
	// ���������JPEG�f�[�^���f�R�[�h����悤�ɐݒ�
	jpeg_mem_src(&decode, jpeg_data, (unsigned long)data_size);
	
	// �w�b�_�̓ǂݍ���
	if(jpeg_read_header(&decode, TRUE) != JPEG_HEADER_OK)
	{
		return NULL;
	}

	// �摜�̕��A�����A�`�����l�������擾
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

	// �s�N�Z���f�[�^�̃��������m��
	pixels = (uint8*)MEM_ALLOC_FUNC(local_height * stride);
	pixel_datas = (uint8**)MEM_ALLOC_FUNC(sizeof(*pixel_datas) * local_height);
	for(i=0; i<local_height; i++)
	{
		pixel_datas[i] = &pixels[i*stride];
	}

	// �f�R�[�h�J�n
	(void)jpeg_start_decompress(&decode);
	// �f�[�^���c���Ă�����胋�[�v
	while(decode.output_scanline < decode.image_height)
	{
		jpeg_read_scanlines(&decode, pixel_datas + decode.output_scanline,
			decode.image_height - decode.output_scanline);
	}

	// �������J��
	jpeg_finish_decompress(&decode);
	MEM_FREE_FUNC(pixel_datas);
	jpeg_destroy_decompress(&decode);
	MEM_FREE_FUNC(jpeg_data);

	return pixels;
}

#ifdef __cplusplus
}
#endif
