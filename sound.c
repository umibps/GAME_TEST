#include <string.h>
#include "sound.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeSoundContext関数
 音声再生に必要な基本的なデータを初期化
 引数
 context	: 音声再生に必要な基本的なデータ
*/
void InitializeSoundContext(SOUND_CONTEXT* context)
{
	(void)memset(context, 0, sizeof(*context));

	context->device = alcOpenDevice(NULL);
	context->context = alcCreateContext(context->device, NULL);
}

/*
 ReleaseSoundContext関数
 音声再生に必要な基本的なデータを開放
 引数
 context	: 音声再生に必要な基本的なデータ
*/
void ReleaseSoundContext(SOUND_CONTEXT* context)
{
	alcDestroyContext(context->context);
	context->context = NULL;
	alcCloseDevice(context->device);
	context->device =  NULL;
}

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
void InitializeSoundPlayBase(
	SOUND_PLAY_BASE* sound_play,
	SOUND_CONTEXT* context,
	size_t(*get_data_func)(uint8* data, size_t size, SOUND_PLAY_BASE* sound_play),
	int(*time_seek_func)(SOUND_PLAY_BASE* sound_play, FLOAT_T time_value),
	void(*delete_func)(void* stream),
	ALenum format,
	ALsizei sample_rate,
	unsigned int play_flags
)
{
	size_t size;
	int i;

	(void)memset(sound_play, 0, sizeof(*sound_play));

	sound_play->context = context;
	sound_play->format = format;
	sound_play->sample_rate = sample_rate;
	sound_play->play_flags = play_flags;

	sound_play->get_data_func = get_data_func;
	sound_play->time_seek_func = time_seek_func;
	sound_play->delete_func = delete_func;

	// OpenALの現在のコンテキストを変更
	alcMakeContextCurrent(context->context);

	// 音声ソースを作成
	alGenSources(1, &sound_play->source);

	// 音声の保管場所を作成
	sound_play->sound_data = (uint8*)MEM_ALLOC_FUNC(SOUND_BUFFER_SIZE);

	// バッファを作成して音声データを読み込む
	for(i=0; i<SOUND_BUFFER_NUM; i++)
	{
		// 音声データを取得
		size = get_data_func(sound_play->sound_data,
			SOUND_BUFFER_SIZE, sound_play);

		// 音声データがなければ終了
		if(size == 0)
		{
			break;
		}

		// OpenALのバッファを作成して
		alGenBuffers(1, &sound_play->buffer[i]);
		// バッファにデータを入れる
		alBufferData(sound_play->buffer[i], sound_play->format,
			sound_play->sound_data, size, sound_play->sample_rate);
		// 再生待ち行列に追加
		alSourceQueueBuffers(sound_play->source, 1, &sound_play->buffer[i]);

		sound_play->queue_size++;
	}
}

/*
 ReleaseSoundPlay関数
 音声再生用の基本的なデータを開放する
 引数
 sound_play	: 音声再生用の基本的なデータ
*/
void ReleaseSoundPlay(SOUND_PLAY_BASE* sound_play)
{
	int i;

	// 再生を停止する
	alcMakeContextCurrent(sound_play->context->context);
	alSourceStop(sound_play->source);

	// OpenALのバッファを破棄する
	for(i=0; i<SOUND_BUFFER_NUM; i++)
	{
		if(sound_play->buffer[i] != 0)
		{
			alDeleteBuffers(1, &sound_play->buffer[i]);
		}
	}

	// OpenALのソースを破棄する
	if(sound_play->source != 0)
	{
		alDeleteSources(1, &sound_play->source);
	}

	// デコードしたデータを入れるバッファを削除
	MEM_FREE_FUNC(sound_play->sound_data);

	// コンテキストをリセット
	alcMakeContextCurrent(NULL);
	
	if(sound_play->delete_func != NULL)
	{
		sound_play->delete_func(sound_play);
	}
}

/*
 PlaySound関数
 音声再生を実行する
 引数
 sound_play	: 音声再生用の基本的なデータ
*/
void PlaySound(SOUND_PLAY_BASE* sound_play)
{
	ALint finished;				// 再生が終了したか判定用
	int second_loop = FALSE;	// ループ再生での嵌り回避用

	if((sound_play->play_flags & SOUND_PLAY_FLAG_PAUSE) != 0)
	{	// 一時停止中なら終了
		return;
	}

	// コンテキストをセット
	alcMakeContextCurrent(sound_play->context->context);

	// 音声ソースの状態を取得
	alGetSourcei(sound_play->source, AL_SOURCE_STATE, &sound_play->play_state);

	// 再生状態でなければ再生開始
	if(sound_play->play_state != AL_PLAYING &&
		(sound_play->play_state != AL_STOPPED || (sound_play->play_flags & SOUND_PLAY_FLAG_LOOP_PLAY) != 0))
	{
		alSourcePlay(sound_play->source);
	}

	// 音声ソースの中に再生終了しているバッファが無いか確認
	alGetSourcei(sound_play->source, AL_BUFFERS_PROCESSED, &finished);

	if(finished > 0)
	{	// 終了しているバッファがある
		size_t size;		// 読み込んだ音声データのバイト数
		int buffer_index;	// 再生が終了したバッファのインデックス
		int i;

		for(i=0; i<finished; i++)
		{
			alSourceUnqueueBuffers(sound_play->source, 1,
				&sound_play->buffer[sound_play->next_unqueue]);
			buffer_index = sound_play->next_unqueue;
			sound_play->next_unqueue = (sound_play->next_unqueue + 1) % sound_play->queue_size;
			size = sound_play->get_data_func(sound_play->sound_data, SOUND_BUFFER_SIZE, sound_play);

			// 音声データを取得できなければ
			if(size == 0 && second_loop == FALSE)
			{
				if((sound_play->play_flags & SOUND_PLAY_FLAG_LOOP_PLAY) != 0)
				{	// ループ再生なら
					// ループ開始位置に移動
					sound_play->time_seek_func(sound_play, sound_play->loop_start_time);

					second_loop = TRUE;

					// 読み込みやり直し
					size = sound_play->get_data_func(sound_play->sound_data, SOUND_BUFFER_SIZE, sound_play);
				}
				else
				{
					if(sound_play->play_state != AL_PLAYING)
					{	// 再生終了
						sound_play->play_flags |= SOUND_PLAY_FLAG_END;
					}

					return;
				}
			}
			

			// バッファに取得したデータを渡す
			alBufferData(sound_play->buffer[buffer_index], sound_play->format,
				sound_play->sound_data, size, sound_play->sample_rate);
			// バッファを再生待ち行列に追加
			alSourceQueueBuffers(sound_play->source, 1, &sound_play->buffer[buffer_index]);

			second_loop = FALSE;
		}
	}
}

/*
 PauseSound関数
 音声再生を一時停止する
 引数
 sound_play	: 音声再生用の基本的なデータ
*/
void PauseSound(SOUND_PLAY_BASE* sound)
{
	alSourcePause(sound->source);
	sound->play_flags |= SOUND_PLAY_FLAG_PAUSE;
}

/*
 StartPlaySound関数
 音声再生を開始する
 sound_play	: 音声再生用の基本的なデータ
*/
void StartPlaySound(SOUND_PLAY_BASE* sound)
{
	sound->play_flags &= ~(SOUND_PLAY_FLAG_PAUSE);
}

/*
 GetWaveSoundData関数
 無圧縮Waveサウンドデータを指定したバイト数取得する
 引数
 buffer		: データを入れるバッファ
 size		: 取得するバイト数
 sound_play	: 無圧縮Waveサウンドデータを再生するためのデータ
 返り値
	取得したバイト数
*/
static size_t GetWaveSoundData(
	void* buffer,
	size_t size,
	WAVE_SOUND_PLAY* sound_play
)
{
	return sound_play->read_func(buffer, 1, size, sound_play->stream);
}

/*
 TimeSeekWaveSound関数
 指定した秒数の位置に移動する
 引数
 sound_play	: 無圧縮Waveサウンドデータを再生するためのデータ
 time_value	: 移動する秒数
 返り値
	成功:0	失敗:0以外
*/
static int TimeSeekWaveSound(
	WAVE_SOUND_PLAY* sound_play,
	FLOAT_T time_value
)
{
	int offset = (int)(sound_play->base_data.sample_rate * sound_play->channel * sound_play->bit * time_value) / 8;
	int byte_per_sample = sound_play->channel * (sound_play->bit / 8);
	int mod = offset % byte_per_sample;
	if(mod > 0)
	{
		offset += byte_per_sample - mod;
	}
	return sound_play->seek_func(sound_play->stream, sound_play->data_top + offset, SEEK_SET);
}

static void OnDeleteWaveSound(WAVE_SOUND_PLAY* sound_play)
{
	if(sound_play->delete_func != NULL)
	{
		sound_play->delete_func(sound_play->stream);
	}
}

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
int InitializeWaveSoundPlay(
	WAVE_SOUND_PLAY* sound_play,
	SOUND_CONTEXT* context,
	void* stream,
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	void (*delete_func)(void*),
	unsigned int play_flags
)
{
	int read_flag;
	uint8 buffer[32];
	int32 data32;
	unsigned int chunk;
	unsigned int sample_rate;
	unsigned int byte_per_second;
	long data_position = 0;
	uint16 block_size;
	int chunk_size;
	ALenum format = 0;

	(void)memset(sound_play, 0, sizeof(*sound_play));

	sound_play->stream = stream;

	sound_play->read_func = read_func;
	sound_play->seek_func = seek_func;
	sound_play->delete_func = delete_func;

	// 'R''I''F''F'をチェック
	if(read_func(buffer, 1, 4, stream) == 4)
	{
		if(buffer[0] != 'R' || buffer[1] != 'I'
			|| buffer[2] != 'F' || buffer[3]!= 'F')
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	// データサイズを取得
	if(read_func(&data32, sizeof(data32), 1, stream) == 0)
	{
		return FALSE;
	}
	sound_play->data_size = data32;

	// WAVEヘッダーの読み込み
	if(read_func(buffer, 1, 4, stream) == 4)
	{
		if(buffer[0] != 'W' || buffer[1] != 'A'
			|| buffer[2] != 'V' || buffer[3] != 'E')
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	// チャンクの読み込み
	read_flag = 0;
	while(read_flag != 3)
	{
		if(read_func(buffer, 1, 4, stream) != 4)
		{
			return FALSE;
		}
		chunk = (buffer[3] << 24) | (buffer[2] << 16)
			| (buffer[1] << 8) | buffer[0];
		if(read_func(buffer, 1, 4, stream) != 4)
		{
			return FALSE;
		}
		chunk_size = (buffer[3] << 24) | (buffer[2] << 16)
			| (buffer[1] << 8) | buffer[0];

		switch(chunk)
		{
		case 0x20746d66:	// フォーマット
			if(read_func(buffer, 1, 2, stream) != 2)
			{
				return FALSE;
			}
			if(buffer[0] != 1 || buffer[1] != 0)
			{
				return FALSE;
			}
			// モノラル or ステレオ
			if(read_func(buffer, 1, 2, stream) != 2)
			{
				return FALSE;
			}
			if(buffer[0] > 2 || buffer[1] != 0)
			{
				return FALSE;
			}
			sound_play->channel = buffer[0];

			// サンプリングレート
			if(read_func(buffer, 1, 4, stream) != 4)
			{
				return FALSE;
			}
			sample_rate = (buffer[3] << 24) | (buffer[2] << 16)
				| (buffer[1] << 8) | buffer[0];

			// 1秒あたりのバイト数
			if(read_func(buffer, 1, 4, stream) != 4)
			{
				return FALSE;
			}
			byte_per_second = (buffer[3] << 24) | (buffer[2] << 16)
				| (buffer[1] << 8) | buffer[0];

			// データブロックのサイズ
			if(read_func(buffer, 1, 2, stream) != 2)
			{
				return FALSE;
			}
			block_size = (buffer[1] << 8) | buffer[0];

			// 1サンプルあたりのビット数
			if(read_func(buffer, 1, 2, stream) != 2)
			{
				return FALSE;
			}
			sound_play->bit = (buffer[1] << 8) | buffer[0];

			read_flag++;

			break;
		case 0x61746164:	// 音声データ
			sound_play->data_size = chunk_size;
			data_position = tell_func(stream);
			(void)seek_func(stream, (long)chunk_size, SEEK_CUR);

			read_flag += 2;
			break;
		default:
			(void)seek_func(stream, (long)chunk_size, SEEK_CUR);
		}
	}

	// フォーマットがデータより後ろにあった場合、戻す
	if((long)data_position != tell_func(stream))
	{
		(void)seek_func(stream, data_position, SEEK_SET);
	}
	sound_play->data_top = (int)data_position;

	// チャンネル数とビット数に応じてフォーマットを決定
	if(sound_play->channel == 1)
	{
		format = (sound_play->bit == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
	}
	else
	{
		format = (sound_play->bit == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
	}

	InitializeSoundPlayBase(
		&sound_play->base_data,
		context,
		(size_t (*)(uint8*, size_t, SOUND_PLAY_BASE*))GetWaveSoundData,
		(int(*)(SOUND_PLAY_BASE*, FLOAT_T))TimeSeekWaveSound,
		NULL,
		format,
		sample_rate,
		play_flags
	);

	return TRUE;
}

/*
 GetVorbisSoundData関数
 Vorbisサウンドデータから波形データを指定したバイト数取得する
 引数
 buffer		: データを入れるバッファ
 size		: 取得するバイト数
 sound_play	: Vorbisサウンドデータを再生するためのデータ
 返り値
	取得したバイト数
*/
static size_t GetVorbisSoundData(
	void* buffer,
	size_t size,
	VORBIS_SOUND_PLAY* sound_play
)
{
	const int endian_detect = 0x01;
	const int endian = ((*(char*)&endian_detect) != 0) ? 0 : 1;
	return ov_read(&sound_play->vorbis_file, (char*)buffer,
		size, endian, 2, 1, &sound_play->current_position);
}

/*
 TimeSeekVorbisSound関数
 指定した秒数の位置に移動する
 引数
 sound_play	: Vorbisサウンドデータを再生するためのデータ
 time_value	: 移動する秒数
 返り値
	成功:0	失敗:0以外
*/
static int TimeSeekVorbisSound(
	VORBIS_SOUND_PLAY* sound_play,
	FLOAT_T time_value
)
{
	return ov_time_seek(&sound_play->vorbis_file, time_value);
}

static void OnDeleteVorbisSound(VORBIS_SOUND_PLAY* sound_play)
{
	(void)ov_clear(&sound_play->vorbis_file);

	if(sound_play->delete_func != NULL)
	{
		sound_play->delete_func(sound_play->stream);
	}
}

static size_t VorbisReadCallback(void* buffer, size_t block_size, size_t num_blocks, VORBIS_SOUND_PLAY* sound_play)
{
	return sound_play->read_func(buffer, block_size, num_blocks, sound_play->stream);
}

static int VorbisSeekCallback(VORBIS_SOUND_PLAY* sound_play, ogg_int64_t offset, int whence)
{
	return sound_play->seek_func(sound_play->stream, (long)offset, whence);
}

static long VorbisTellCallback(VORBIS_SOUND_PLAY* sound_play)
{
	return sound_play->tell_func(sound_play->stream);
}

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
int InitializeVorbisSoundPlay(
	VORBIS_SOUND_PLAY* sound_play,
	SOUND_CONTEXT* context,
	void* stream,
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	void (*delete_func)(void*),
	unsigned int play_flags
)
{
	ov_callbacks callbacks ={
		(size_t (*)(void*, size_t, size_t, void*))VorbisReadCallback,
		(int (*)(void*, ogg_int64_t, int))VorbisSeekCallback,
		NULL,
		(long (*)(void*))VorbisTellCallback
	};
	ALenum format;

	(void)memset(sound_play, 0, sizeof(*sound_play));

	sound_play->stream = stream;

	sound_play->read_func = read_func;
	sound_play->seek_func = seek_func;
	sound_play->tell_func = tell_func;
	sound_play->delete_func = delete_func;

	if(ov_open_callbacks(sound_play, &sound_play->vorbis_file, NULL, 0, callbacks))
	{
		return FALSE;
	}

	// 音声のサンプリングレートやモノラルorステレオの情報を取得
	sound_play->vorbis_info = ov_info(&sound_play->vorbis_file, -1);
	format = (sound_play->vorbis_info->channels == 1)
		? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	InitializeSoundPlayBase(
		&sound_play->base_data,
		context,
		(size_t (*)(uint8*, size_t, SOUND_PLAY_BASE*))GetVorbisSoundData,
		(int(*)(SOUND_PLAY_BASE*, FLOAT_T))TimeSeekVorbisSound,
		(void (*)(SOUND_PLAY_BASE*))OnDeleteVorbisSound,
		format,
		sound_play->vorbis_info->rate,
		play_flags
	);

	return TRUE;
}

/*
 GetFlacSoundData関数
 FLACサウンドデータから波形データを指定したバイト数取得する
 引数
 buffer		: データを入れるバッファ
 size		: 取得するバイト数
 sound_play	: FLACサウンドデータを再生するためのデータ
 返り値
	取得したバイト数
*/
static size_t GetFlacSoundData(
	void* buffer,
	size_t size,
	FLAC_SOUND_PLAY* sound_play
)
{
	// 前回の残りデータが要求サイズより少ないなら
	if(sound_play->carry_size < (int)size)
	{	// 残りのデータをバッファにコピー
		(void)memcpy(buffer, sound_play->carry_data, sound_play->carry_size);
		// データを新たにデコードする
		sound_play->write_position = sound_play->carry_size;
		sound_play->carry_size = 0;
		while(sound_play->write_position < (int)size
			&& sound_play->tell_func(sound_play->stream) != sound_play->data_size)
		{	// データの終端までデコード
			FLAC__stream_decoder_process_single(sound_play->decoder);
		}
	}
	else
	{	// 前回の残りデータが要求サイズより大きいので
			// 残りデータをコピーして
		(void)memcpy(buffer, sound_play->carry_data, size);
		// コピーし終わったデータは破棄
		(void)memmove(sound_play->carry_data, &sound_play->carry_data[size],
			sound_play->carry_size - size);
		sound_play->carry_size -= size;
		sound_play->write_position = size;
	}

	return sound_play->write_position;
}

/*
 TimeSeekFlacSound関数
 指定した秒数の位置に移動する
 引数
 sound_play	: FLACサウンドデータを再生するためのデータ
 time_value	: 移動する秒数
 返り値
	成功:0	失敗:0以外
*/
static int TimeSeekFlacSound(
	FLAC_SOUND_PLAY* sound_play,
	FLOAT_T time_value
)
{
	int sample = (int)(sound_play->base_data.sample_rate * time_value);
	return !(FLAC__stream_decoder_seek_absolute(sound_play->decoder, sample));
}

static void OnDeleteFlacSound(FLAC_SOUND_PLAY* sound_play)
{
	FLAC__stream_decoder_delete(sound_play->decoder);

	if(sound_play->delete_func != NULL)
	{
		sound_play->delete_func(sound_play->stream);
	}
}

static FLAC__StreamDecoderReadStatus FlacReadCallback(
	const FLAC__StreamDecoder* decoder,
	FLAC__byte* buffer,
	size_t* bytes,
	FLAC_SOUND_PLAY* sound_play
)
{
	if(*bytes > 0)
	{
		*bytes = sound_play->read_func(buffer, 1, *bytes, sound_play->stream);
		if(*bytes == 0)
		{
			return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		}
		return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
	}

	return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}

static FLAC__StreamDecoderSeekStatus FlacSeekCallback(
	const FLAC__StreamDecoder* decoder,
	FLAC__uint64 offset,
	FLAC_SOUND_PLAY* sound_play
)
{
	if(sound_play->seek_func(sound_play->stream, (long)offset, SEEK_SET) < 0)
	{
		return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
	}
	return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

static FLAC__StreamDecoderTellStatus FlacTellCallback(
	const FLAC__StreamDecoder* decoder,
	FLAC__uint64* offset,
	FLAC_SOUND_PLAY* sound_play
)
{
	*offset = sound_play->tell_func(sound_play->stream);
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

static FLAC__StreamDecoderLengthStatus FlacLengthCallback(
	const FLAC__StreamDecoder* decoder,
	FLAC__uint64* length,
	FLAC_SOUND_PLAY* sound_play
	)
{
	*length = sound_play->data_size;
	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

static FLAC__bool FlacEndOfFileCallback(const FLAC__StreamDecoder* decoder, FLAC_SOUND_PLAY* sound_play)
{
	return sound_play->tell_func(sound_play->stream) == sound_play->data_size;
}

static void FlacMetadataCallback(
	const FLAC__StreamDecoder* decoder,
	const FLAC__StreamMetadata* metadata,
	FLAC_SOUND_PLAY* sound_play
)
{
	if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
	{
		ALenum format;

		sound_play->bits_per_sample = metadata->data.stream_info.bits_per_sample;
		sound_play->channels = metadata->data.stream_info.channels;

		// OpenAL周りの初期化
		format = (metadata->data.stream_info.channels == 1)
			? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

		InitializeSoundPlayBase(
			&sound_play->base_data,
			sound_play->base_data.context,
			(size_t (*)(uint8*, size_t, SOUND_PLAY_BASE*))GetFlacSoundData,
			(int(*)(SOUND_PLAY_BASE*, FLOAT_T))TimeSeekFlacSound,
			(void (*)(SOUND_PLAY_BASE*))OnDeleteFlacSound,
			format,
			metadata->data.stream_info.sample_rate,
			sound_play->base_data.play_flags
		);
	}
}

static FLAC__StreamDecoderWriteStatus FlacWriteCallback(
	const FLAC__StreamDecoder* decoder,
	const FLAC__Frame* frame,
	const FLAC__int32* const buffer[],
	FLAC_SOUND_PLAY* sound_play
)
{
	unsigned int num_blocks;
	unsigned int block_size = sound_play->bits_per_sample / 8 * sound_play->channels;
	unsigned int carry_size = 0;
	unsigned int i;

	num_blocks = frame->header.blocksize;
	// バッファオーバーランチェック
	if(num_blocks * block_size + sound_play->write_position > SOUND_BUFFER_SIZE)
	{	// このままではバッファオーバーランするので一時保管場所に入れるサイズを決める
		carry_size = num_blocks * block_size + sound_play->write_position - SOUND_BUFFER_SIZE;
		carry_size /= block_size;
		sound_play->carry_size = carry_size * block_size;
		num_blocks = frame->header.blocksize - carry_size;
	}
	else
	{	// バッファオーバーラン無し
		num_blocks = frame->header.blocksize;
	}

	// ステレオorモノラル & 音声のサンプルサイズに合わせて処理を切り替え
	switch(sound_play->channels)
	{
	case 1:
		switch(sound_play->bits_per_sample)
		{
		case 8:
			for(i=0; i<num_blocks; i++)
			{
				sound_play->base_data.buffer[i+sound_play->write_position] = (uint8)buffer[0][i];
			}
			for(i=0; i<carry_size; i++)
			{
				sound_play->carry_data[i] = (uint8)buffer[0][num_blocks+i];
			}
			break;
		case 16:
			{
				uint16 *dst;
				dst = (uint16*)&sound_play->base_data.sound_data[sound_play->write_position];
				for(i=0; i<num_blocks; i++)
				{
					dst[i] = (uint16)buffer[0][i];
				}
				dst = (uint16*)sound_play->carry_data;
				for(i=0; i<carry_size; i++)
				{
					((uint16*)sound_play->carry_data)[i] = (uint16)buffer[0][num_blocks+i];
				}
			}
			break;
		}
		break;
	case 2:
		switch(sound_play->bits_per_sample)
		{
		case 8:
			for(i=0; i<num_blocks; i++)
			{
				sound_play->base_data.buffer[i*2+0+sound_play->write_position] = (uint8)buffer[0][i];
				sound_play->base_data.buffer[i*2+1+sound_play->write_position] = (uint8)buffer[1][i];
			}
			for(i=0; i<carry_size; i++)
			{
				sound_play->carry_data[i*2+0] = (uint8)buffer[0][num_blocks+i];
				sound_play->carry_data[i*2+1] = (uint8)buffer[1][num_blocks+i];
			}
			break;
		case 16:
			{
				uint16 *dst;
				dst = (uint16*)&sound_play->base_data.sound_data[sound_play->write_position];
				for(i=0; i<num_blocks; i++)
				{
					dst[i*2+0] = (uint16)buffer[0][i];
					dst[i*2+1] = (uint16)buffer[1][i];
				}
				dst = (uint16*)sound_play->carry_data;
				for(i=0; i<carry_size; i++)
				{
					dst[i*2+0] = (uint16)buffer[0][num_blocks+i];
					dst[i*2+1] = (uint16)buffer[1][num_blocks+i];
				}
			}
			break;
		}
		break;
	}

	sound_play->write_position += num_blocks * block_size;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void FlacErrorCallback(
	const FLAC__StreamDecoder* decoder,
	FLAC__StreamDecoderErrorStatus status,
	FLAC_SOUND_PLAY* sound_play
)
{
}

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
int InitializeFlacSoundPlay(
	FLAC_SOUND_PLAY* sound_play,
	SOUND_CONTEXT* context,
	void* stream,
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	void (*delete_func)(void*),
	unsigned int play_flags
)
{

	(void)memset(sound_play, 0, sizeof(*sound_play));

	sound_play->stream = stream;

	sound_play->read_func = read_func;
	sound_play->seek_func = seek_func;
	sound_play->tell_func = tell_func;
	sound_play->delete_func = delete_func;

	(void)seek_func(stream, 0, SEEK_END);
	sound_play->data_size = tell_func(stream);
	(void)seek_func(stream, 0, SEEK_SET);

	sound_play->base_data.context = context;
	sound_play->base_data.play_flags = play_flags;

	sound_play->decoder = FLAC__stream_decoder_new();

	if(FLAC__stream_decoder_init_stream(
		sound_play->decoder,
		(FLAC__StreamDecoderReadCallback)FlacReadCallback,
		(FLAC__StreamDecoderSeekCallback)FlacSeekCallback,
		(FLAC__StreamDecoderTellCallback)FlacTellCallback,
		(FLAC__StreamDecoderLengthCallback)FlacLengthCallback,
		(FLAC__StreamDecoderEofCallback)FlacEndOfFileCallback,
		(FLAC__StreamDecoderWriteCallback)FlacWriteCallback,
		(FLAC__StreamDecoderMetadataCallback)FlacMetadataCallback,
		(FLAC__StreamDecoderErrorCallback)FlacErrorCallback,
		sound_play)
			!= FLAC__STREAM_DECODER_INIT_STATUS_OK
	)
	{
		FLAC__stream_decoder_delete(sound_play->decoder);
		return FALSE;
	}

	if(FLAC__stream_decoder_process_until_end_of_metadata(sound_play->decoder) == FALSE)
	{
		FLAC__stream_decoder_delete(sound_play->decoder);
		return FALSE;
	}
	
	return TRUE;
}

/*
 InitiaizeSounds関数
 BGM, 効果音全体を管理する構造体を初期化
 引数
 sounds		: BGM, 効果音全体を管理する構造体
 context	: 音声再生用の基本データ
*/
void InitializeSounds(SOUNDS* sounds, SOUND_CONTEXT* context)
{
	(void)memset(sounds, 0, sizeof(*sounds));

	sounds->sounds = (SOUND_ITEM**)MEM_ALLOC_FUNC(
		sizeof(*sounds->sounds) * SOUNDS_DEFAULT_BUFFER_SIZE);
	(void)memset(sounds->sounds, 0, sizeof(*sounds->sounds) * SOUNDS_DEFAULT_BUFFER_SIZE);
	sounds->buffer_size = SOUNDS_DEFAULT_BUFFER_SIZE;
	sounds->context = context;
}

static size_t ReadSoundData(void* buffer, size_t block_size, size_t num_blocks, SOUND_ITEM* sound_item)
{
	return sound_item->read_func(buffer, block_size, num_blocks, sound_item->stream);
}

static int SeekSoundData(SOUND_ITEM* sound_item, long offset, int whence)
{
	return sound_item->seek_func(sound_item->stream, offset, whence);
}

static int TellSoundData(SOUND_ITEM* sound_item)
{
	return sound_item->tell_func(sound_item->stream);
}

static void OnDeleteSoundData(SOUND_ITEM* sound_item)
{
	size_t i;
	for(i=0; i<sound_item->sounds->buffer_size; i++)
	{
		if(sound_item->sounds->sounds[i] == sound_item)
		{
			sound_item->sounds->sounds[i] = NULL;
			if(sound_item->sounds->search_start > i)
			{
				sound_item->sounds->search_start = i;
			}
		}
	}

	if(sound_item->delete_func != NULL)
	{
		sound_item->delete_func(sound_item->stream);
	}
	
	if(sound_item->sounds->num_sounds > 0)
	{
		sound_item->sounds->num_sounds--;
	}

	MEM_FREE_FUNC(sound_item->sound_play);
	MEM_FREE_FUNC(sound_item);
}

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
SOUND_PLAY_BASE* AddSound(
	SOUNDS* sounds,
	const char* path,
	void* (*open_func)(const char*, const char*, void*),
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	void (*delete_func)(void*),
	unsigned int play_flags,
	void* user_data
)
{
	SOUND_PLAY_BASE *ret = NULL;
	size_t search_start = sounds->search_start;
	void *stream;
	const char *extention = path;
	SOUND_ITEM **target = NULL;

	if((stream = open_func(path, "rb", user_data)) == NULL)
	{	// ファイルオープン失敗
		return NULL;
	}

	// 拡張子を取得
	{
		const char *p = extention;
		while(*p != '\0')
		{
			if(*p == '.')
			{
				extention = p;
			}
			p = GetNextUtf8Character(p);
		}
	}

	for( ; search_start<sounds->buffer_size; search_start++)
	{
		if(sounds->sounds[search_start] == NULL)
		{
			target = &sounds->sounds[search_start];
			break;
		}
	}
	if(target == NULL)
	{
		delete_func(stream);
		return NULL;
	}

	*target = (SOUND_ITEM*)MEM_ALLOC_FUNC(sizeof(**target));
	(*target)->stream = stream;
	(*target)->read_func = read_func;
	(*target)->seek_func = seek_func;
	(*target)->tell_func = tell_func;
	(*target)->delete_func = delete_func;

	if(target == NULL)
	{
		return NULL;
	}

	if(StringCompareIgnoreCase(extention, ".ogg") == 0
		|| StringCompareIgnoreCase(extention, ".oga") == 0)
	{
		ret = (SOUND_PLAY_BASE*)MEM_ALLOC_FUNC(sizeof(VORBIS_SOUND_PLAY));
		if(InitializeVorbisSoundPlay(
			(VORBIS_SOUND_PLAY*)ret,
			sounds->context,
			*target,
			(size_t(*)(void*, size_t, size_t, void*))ReadSoundData,
			(int(*)(void*, long, int))SeekSoundData,
			(long(*)(void*))TellSoundData,
			(void(*)(void*))OnDeleteSoundData,
			play_flags
			) == FALSE
		)
		{
			delete_func(stream);
			return NULL;
		}
	}
	else if(StringCompareIgnoreCase(extention, ".flac") == 0)
	{
		ret = (SOUND_PLAY_BASE*)MEM_ALLOC_FUNC(sizeof(FLAC_SOUND_PLAY));
		if(InitializeFlacSoundPlay(
			(FLAC_SOUND_PLAY*)ret,
			sounds->context,
			*target,
			(size_t(*)(void*, size_t, size_t, void*))ReadSoundData,
			(int(*)(void*, long, int))SeekSoundData,
			(long(*)(void*))TellSoundData,
			(void(*)(void*))OnDeleteSoundData,
			play_flags
			) == FALSE
		)
		{
			delete_func(stream);
			return NULL;
		}
	}
	else if(StringCompareIgnoreCase(extention, ".wav") == 0)
	{
		ret = (SOUND_PLAY_BASE*)MEM_ALLOC_FUNC(sizeof(WAVE_SOUND_PLAY));
		if(InitializeWaveSoundPlay(
			(WAVE_SOUND_PLAY*)ret,
			sounds->context,
			*target,
			(size_t(*)(void*, size_t, size_t, void*))ReadSoundData,
			(int(*)(void*, long, int))SeekSoundData,
			(long(*)(void*))TellSoundData,
			(void(*)(void*))OnDeleteSoundData,
			play_flags
			) == FALSE
		)
		{
			delete_func(stream);
			return NULL;
		}
	}
	else
	{
		delete_func(stream);
		return NULL;
	}
	(*target)->sound_play = ret;
	(*target)->sounds = sounds;

	sounds->search_start = search_start;
	sounds->num_sounds++;

	if(sounds->num_sounds >= sounds->buffer_size)
	{
		size_t before_size = sounds->buffer_size;
		sounds->buffer_size += SOUNDS_DEFAULT_BUFFER_SIZE;
		sounds->sounds = (SOUND_ITEM**)MEM_REALLOC_FUNC(
			sounds->sounds, sizeof(*sounds->sounds) * sounds->buffer_size);
		(void)memset(&sounds[before_size], 0, sizeof(*sounds->sounds) * SOUNDS_DEFAULT_BUFFER_SIZE);
		sounds->search_start = 0;
	}

	return ret;
}

/*
 PlayAllSound関数
 全てのBGM, 効果音を再生する
 引数
 sounds	: BGM, 効果音全体を管理する構造体
*/
void PlayAllSound(SOUNDS* sounds)
{
	size_t num_play;
	size_t i;

	for(num_play=0, i=0; i<sounds->buffer_size && num_play<sounds->num_sounds; i++)
	{
		if(sounds->sounds[i] != NULL)
		{
			PlaySound(sounds->sounds[i]->sound_play);
			if((sounds->sounds[i]->sound_play->play_flags & SOUND_PLAY_FLAG_END) != 0
				&& (sounds->sounds[i]->sound_play->play_flags & SOUND_PLAY_FLAG_LOOP_PLAY) == 0)
			{
				ReleaseSoundPlay(sounds->sounds[i]->sound_play);
			}
			else
			{
				num_play++;
			}
		}
	}
}

#ifdef __cplusplus
}
#endif
