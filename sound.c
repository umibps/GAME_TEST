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

	if(sound_play->delete_func != NULL)
	{
		sound_play->delete_func(sound_play);
	}

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

	// コンテキストをセット
	alcMakeContextCurrent(sound_play->context->context);

	// 音声ソースの状態を取得
	alGetSourcei(sound_play->source, AL_SOURCE_STATE, &sound_play->play_state);

	// 再生状態でなければ再生開始
	if(sound_play->play_state != AL_PLAYING && sound_play->play_state != AL_STOPPED)
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
					continue;
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

#ifdef __cplusplus
}
#endif
