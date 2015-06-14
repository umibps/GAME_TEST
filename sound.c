#include <string.h>
#include "sound.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeSoundContext�֐�
 �����Đ��ɕK�v�Ȋ�{�I�ȃf�[�^��������
 ����
 context	: �����Đ��ɕK�v�Ȋ�{�I�ȃf�[�^
*/
void InitializeSoundContext(SOUND_CONTEXT* context)
{
	(void)memset(context, 0, sizeof(*context));

	context->device = alcOpenDevice(NULL);
	context->context = alcCreateContext(context->device, NULL);
}

/*
 ReleaseSoundContext�֐�
 �����Đ��ɕK�v�Ȋ�{�I�ȃf�[�^���J��
 ����
 context	: �����Đ��ɕK�v�Ȋ�{�I�ȃf�[�^
*/
void ReleaseSoundContext(SOUND_CONTEXT* context)
{
	alcDestroyContext(context->context);
	context->context = NULL;
	alcCloseDevice(context->device);
	context->device =  NULL;
}

/*
 InitializeSoundPlayBase�֐�
 �����Đ��p�̊�{�f�[�^��������
 ����
 sound_play		: �����Đ��p�̊�{�I�ȃf�[�^
 context		: �����Đ��p�̃R���e�L�X�g
 get_data_func	: �����f�[�^���f�R�[�h���Ď擾����֐��|�C���^
 time_seek_func	: ���ԂōĐ��ʒu���V�[�N���邽�߂̊֐��|�C���^
 delete_func	: �f�[�^�폜���Ɏg���֐��|�C���^
 format			: �����f�[�^�̃t�H�[�}�b�g
 sample_rate	: �����f�[�^�̃T���v�����O���[�g
 play_flags		: �����Đ��̃t���O
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

	// OpenAL�̌��݂̃R���e�L�X�g��ύX
	alcMakeContextCurrent(context->context);

	// �����\�[�X���쐬
	alGenSources(1, &sound_play->source);

	// �����̕ۊǏꏊ���쐬
	sound_play->sound_data = (uint8*)MEM_ALLOC_FUNC(SOUND_BUFFER_SIZE);

	// �o�b�t�@���쐬���ĉ����f�[�^��ǂݍ���
	for(i=0; i<SOUND_BUFFER_NUM; i++)
	{
		// �����f�[�^���擾
		size = get_data_func(sound_play->sound_data,
			SOUND_BUFFER_SIZE, sound_play);

		// �����f�[�^���Ȃ���ΏI��
		if(size == 0)
		{
			break;
		}

		// OpenAL�̃o�b�t�@���쐬����
		alGenBuffers(1, &sound_play->buffer[i]);
		// �o�b�t�@�Ƀf�[�^������
		alBufferData(sound_play->buffer[i], sound_play->format,
			sound_play->sound_data, size, sound_play->sample_rate);
		// �Đ��҂��s��ɒǉ�
		alSourceQueueBuffers(sound_play->source, 1, &sound_play->buffer[i]);

		sound_play->queue_size++;
	}
}

/*
 ReleaseSoundPlay�֐�
 �����Đ��p�̊�{�I�ȃf�[�^���J������
 ����
 sound_play	: �����Đ��p�̊�{�I�ȃf�[�^
*/
void ReleaseSoundPlay(SOUND_PLAY_BASE* sound_play)
{
	int i;

	// �Đ����~����
	alcMakeContextCurrent(sound_play->context->context);
	alSourceStop(sound_play->source);

	// OpenAL�̃o�b�t�@��j������
	for(i=0; i<SOUND_BUFFER_NUM; i++)
	{
		if(sound_play->buffer[i] != 0)
		{
			alDeleteBuffers(1, &sound_play->buffer[i]);
		}
	}

	// OpenAL�̃\�[�X��j������
	if(sound_play->source != 0)
	{
		alDeleteSources(1, &sound_play->source);
	}

	// �f�R�[�h�����f�[�^������o�b�t�@���폜
	MEM_FREE_FUNC(sound_play->sound_data);

	// �R���e�L�X�g�����Z�b�g
	alcMakeContextCurrent(NULL);
	
	if(sound_play->delete_func != NULL)
	{
		sound_play->delete_func(sound_play);
	}
}

/*
 PlaySound�֐�
 �����Đ������s����
 ����
 sound_play	: �����Đ��p�̊�{�I�ȃf�[�^
*/
void PlaySound(SOUND_PLAY_BASE* sound_play)
{
	ALint finished;				// �Đ����I������������p
	int second_loop = FALSE;	// ���[�v�Đ��ł̛Ƃ���p

	if((sound_play->play_flags & SOUND_PLAY_FLAG_PAUSE) != 0)
	{	// �ꎞ��~���Ȃ�I��
		return;
	}

	// �R���e�L�X�g���Z�b�g
	alcMakeContextCurrent(sound_play->context->context);

	// �����\�[�X�̏�Ԃ��擾
	alGetSourcei(sound_play->source, AL_SOURCE_STATE, &sound_play->play_state);

	// �Đ���ԂłȂ���΍Đ��J�n
	if(sound_play->play_state != AL_PLAYING &&
		(sound_play->play_state != AL_STOPPED || (sound_play->play_flags & SOUND_PLAY_FLAG_LOOP_PLAY) != 0))
	{
		alSourcePlay(sound_play->source);
	}

	// �����\�[�X�̒��ɍĐ��I�����Ă���o�b�t�@���������m�F
	alGetSourcei(sound_play->source, AL_BUFFERS_PROCESSED, &finished);

	if(finished > 0)
	{	// �I�����Ă���o�b�t�@������
		size_t size;		// �ǂݍ��񂾉����f�[�^�̃o�C�g��
		int buffer_index;	// �Đ����I�������o�b�t�@�̃C���f�b�N�X
		int i;

		for(i=0; i<finished; i++)
		{
			alSourceUnqueueBuffers(sound_play->source, 1,
				&sound_play->buffer[sound_play->next_unqueue]);
			buffer_index = sound_play->next_unqueue;
			sound_play->next_unqueue = (sound_play->next_unqueue + 1) % sound_play->queue_size;
			size = sound_play->get_data_func(sound_play->sound_data, SOUND_BUFFER_SIZE, sound_play);

			// �����f�[�^���擾�ł��Ȃ����
			if(size == 0 && second_loop == FALSE)
			{
				if((sound_play->play_flags & SOUND_PLAY_FLAG_LOOP_PLAY) != 0)
				{	// ���[�v�Đ��Ȃ�
					// ���[�v�J�n�ʒu�Ɉړ�
					sound_play->time_seek_func(sound_play, sound_play->loop_start_time);

					second_loop = TRUE;

					// �ǂݍ��݂�蒼��
					size = sound_play->get_data_func(sound_play->sound_data, SOUND_BUFFER_SIZE, sound_play);
				}
				else
				{
					if(sound_play->play_state != AL_PLAYING)
					{	// �Đ��I��
						sound_play->play_flags |= SOUND_PLAY_FLAG_END;
					}

					return;
				}
			}
			

			// �o�b�t�@�Ɏ擾�����f�[�^��n��
			alBufferData(sound_play->buffer[buffer_index], sound_play->format,
				sound_play->sound_data, size, sound_play->sample_rate);
			// �o�b�t�@���Đ��҂��s��ɒǉ�
			alSourceQueueBuffers(sound_play->source, 1, &sound_play->buffer[buffer_index]);

			second_loop = FALSE;
		}
	}
}

/*
 PauseSound�֐�
 �����Đ����ꎞ��~����
 ����
 sound_play	: �����Đ��p�̊�{�I�ȃf�[�^
*/
void PauseSound(SOUND_PLAY_BASE* sound)
{
	alSourcePause(sound->source);
	sound->play_flags |= SOUND_PLAY_FLAG_PAUSE;
}

/*
 StartPlaySound�֐�
 �����Đ����J�n����
 sound_play	: �����Đ��p�̊�{�I�ȃf�[�^
*/
void StartPlaySound(SOUND_PLAY_BASE* sound)
{
	sound->play_flags &= ~(SOUND_PLAY_FLAG_PAUSE);
}

/*
 GetWaveSoundData�֐�
 �����kWave�T�E���h�f�[�^���w�肵���o�C�g���擾����
 ����
 buffer		: �f�[�^������o�b�t�@
 size		: �擾����o�C�g��
 sound_play	: �����kWave�T�E���h�f�[�^���Đ����邽�߂̃f�[�^
 �Ԃ�l
	�擾�����o�C�g��
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
 TimeSeekWaveSound�֐�
 �w�肵���b���̈ʒu�Ɉړ�����
 ����
 sound_play	: �����kWave�T�E���h�f�[�^���Đ����邽�߂̃f�[�^
 time_value	: �ړ�����b��
 �Ԃ�l
	����:0	���s:0�ȊO
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
 InitializerWaveSoundPlay�֐�
 �����kWave�T�E���h�f�[�^���Đ����邽�߂̃f�[�^��������
 ����
 sound_play		: �����kWave�T�E���h�f�[�^���Đ����邽�߂̃f�[�^
 context		: �����Đ��p�̃R���e�L�X�g
 stream			: �����f�[�^�X�g���[��
 read_func		: �f�[�^�ǂݍ��ݗp�̊֐��|�C���^
 seek_func		: �f�[�^�V�[�N�p�̊֐��|�C���^
 tell_func		: �X�g���[���̈ʒu�擾�p�̊֐��|�C���^
 delete_func	: �f�[�^�폜���Ɏg���֐��|�C���^
 play_flags		: �Đ��֘A�̃t���O
 �Ԃ�l
	����I��:TRUE	���s:FALSE
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

	// 'R''I''F''F'���`�F�b�N
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

	// �f�[�^�T�C�Y���擾
	if(read_func(&data32, sizeof(data32), 1, stream) == 0)
	{
		return FALSE;
	}
	sound_play->data_size = data32;

	// WAVE�w�b�_�[�̓ǂݍ���
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

	// �`�����N�̓ǂݍ���
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
		case 0x20746d66:	// �t�H�[�}�b�g
			if(read_func(buffer, 1, 2, stream) != 2)
			{
				return FALSE;
			}
			if(buffer[0] != 1 || buffer[1] != 0)
			{
				return FALSE;
			}
			// ���m���� or �X�e���I
			if(read_func(buffer, 1, 2, stream) != 2)
			{
				return FALSE;
			}
			if(buffer[0] > 2 || buffer[1] != 0)
			{
				return FALSE;
			}
			sound_play->channel = buffer[0];

			// �T���v�����O���[�g
			if(read_func(buffer, 1, 4, stream) != 4)
			{
				return FALSE;
			}
			sample_rate = (buffer[3] << 24) | (buffer[2] << 16)
				| (buffer[1] << 8) | buffer[0];

			// 1�b������̃o�C�g��
			if(read_func(buffer, 1, 4, stream) != 4)
			{
				return FALSE;
			}
			byte_per_second = (buffer[3] << 24) | (buffer[2] << 16)
				| (buffer[1] << 8) | buffer[0];

			// �f�[�^�u���b�N�̃T�C�Y
			if(read_func(buffer, 1, 2, stream) != 2)
			{
				return FALSE;
			}
			block_size = (buffer[1] << 8) | buffer[0];

			// 1�T���v��������̃r�b�g��
			if(read_func(buffer, 1, 2, stream) != 2)
			{
				return FALSE;
			}
			sound_play->bit = (buffer[1] << 8) | buffer[0];

			read_flag++;

			break;
		case 0x61746164:	// �����f�[�^
			sound_play->data_size = chunk_size;
			data_position = tell_func(stream);
			(void)seek_func(stream, (long)chunk_size, SEEK_CUR);

			read_flag += 2;
			break;
		default:
			(void)seek_func(stream, (long)chunk_size, SEEK_CUR);
		}
	}

	// �t�H�[�}�b�g���f�[�^�����ɂ������ꍇ�A�߂�
	if((long)data_position != tell_func(stream))
	{
		(void)seek_func(stream, data_position, SEEK_SET);
	}
	sound_play->data_top = (int)data_position;

	// �`�����l�����ƃr�b�g���ɉ����ăt�H�[�}�b�g������
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
 GetVorbisSoundData�֐�
 Vorbis�T�E���h�f�[�^����g�`�f�[�^���w�肵���o�C�g���擾����
 ����
 buffer		: �f�[�^������o�b�t�@
 size		: �擾����o�C�g��
 sound_play	: Vorbis�T�E���h�f�[�^���Đ����邽�߂̃f�[�^
 �Ԃ�l
	�擾�����o�C�g��
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
 TimeSeekVorbisSound�֐�
 �w�肵���b���̈ʒu�Ɉړ�����
 ����
 sound_play	: Vorbis�T�E���h�f�[�^���Đ����邽�߂̃f�[�^
 time_value	: �ړ�����b��
 �Ԃ�l
	����:0	���s:0�ȊO
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
 InitializerVorbisSoundPlay�֐�
 Vorbis�T�E���h�f�[�^���Đ����邽�߂̃f�[�^��������
 ����
 sound_play		: Vorbis�T�E���h�f�[�^���Đ����邽�߂̃f�[�^
 context		: �����Đ��p�̃R���e�L�X�g
 stream			: �����f�[�^�X�g���[��
 read_func		: �f�[�^�ǂݍ��ݗp�̊֐��|�C���^
 seek_func		: �f�[�^�V�[�N�p�̊֐��|�C���^
 tell_func		: �X�g���[���̈ʒu�擾�p�̊֐��|�C���^
 delete_func	: �f�[�^�폜���Ɏg���֐��|�C���^
 play_flags		: �Đ��֘A�̃t���O
 �Ԃ�l
	����I��:TRUE	���s:FALSE
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

	// �����̃T���v�����O���[�g�⃂�m����or�X�e���I�̏����擾
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
 GetFlacSoundData�֐�
 FLAC�T�E���h�f�[�^����g�`�f�[�^���w�肵���o�C�g���擾����
 ����
 buffer		: �f�[�^������o�b�t�@
 size		: �擾����o�C�g��
 sound_play	: FLAC�T�E���h�f�[�^���Đ����邽�߂̃f�[�^
 �Ԃ�l
	�擾�����o�C�g��
*/
static size_t GetFlacSoundData(
	void* buffer,
	size_t size,
	FLAC_SOUND_PLAY* sound_play
)
{
	// �O��̎c��f�[�^���v���T�C�Y��菭�Ȃ��Ȃ�
	if(sound_play->carry_size < (int)size)
	{	// �c��̃f�[�^���o�b�t�@�ɃR�s�[
		(void)memcpy(buffer, sound_play->carry_data, sound_play->carry_size);
		// �f�[�^��V���Ƀf�R�[�h����
		sound_play->write_position = sound_play->carry_size;
		sound_play->carry_size = 0;
		while(sound_play->write_position < (int)size
			&& sound_play->tell_func(sound_play->stream) != sound_play->data_size)
		{	// �f�[�^�̏I�[�܂Ńf�R�[�h
			FLAC__stream_decoder_process_single(sound_play->decoder);
		}
	}
	else
	{	// �O��̎c��f�[�^���v���T�C�Y���傫���̂�
			// �c��f�[�^���R�s�[����
		(void)memcpy(buffer, sound_play->carry_data, size);
		// �R�s�[���I������f�[�^�͔j��
		(void)memmove(sound_play->carry_data, &sound_play->carry_data[size],
			sound_play->carry_size - size);
		sound_play->carry_size -= size;
		sound_play->write_position = size;
	}

	return sound_play->write_position;
}

/*
 TimeSeekFlacSound�֐�
 �w�肵���b���̈ʒu�Ɉړ�����
 ����
 sound_play	: FLAC�T�E���h�f�[�^���Đ����邽�߂̃f�[�^
 time_value	: �ړ�����b��
 �Ԃ�l
	����:0	���s:0�ȊO
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

		// OpenAL����̏�����
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
	// �o�b�t�@�I�[�o�[�����`�F�b�N
	if(num_blocks * block_size + sound_play->write_position > SOUND_BUFFER_SIZE)
	{	// ���̂܂܂ł̓o�b�t�@�I�[�o�[��������̂ňꎞ�ۊǏꏊ�ɓ����T�C�Y�����߂�
		carry_size = num_blocks * block_size + sound_play->write_position - SOUND_BUFFER_SIZE;
		carry_size /= block_size;
		sound_play->carry_size = carry_size * block_size;
		num_blocks = frame->header.blocksize - carry_size;
	}
	else
	{	// �o�b�t�@�I�[�o�[��������
		num_blocks = frame->header.blocksize;
	}

	// �X�e���Ior���m���� & �����̃T���v���T�C�Y�ɍ��킹�ď�����؂�ւ�
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
 InitializerFlacSoundPlay�֐�
 FLAC�T�E���h�f�[�^���Đ����邽�߂̃f�[�^��������
 ����
 sound_play		: FLAC�T�E���h�f�[�^���Đ����邽�߂̃f�[�^
 context		: �����Đ��p�̃R���e�L�X�g
 stream			: �����f�[�^�X�g���[��
 read_func		: �f�[�^�ǂݍ��ݗp�̊֐��|�C���^
 seek_func		: �f�[�^�V�[�N�p�̊֐��|�C���^
 tell_func		: �X�g���[���̈ʒu�擾�p�̊֐��|�C���^
 delete_func	: �f�[�^�폜���Ɏg���֐��|�C���^
 play_flags		: �Đ��֘A�̃t���O
 �Ԃ�l
	����I��:TRUE	���s:FALSE
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
 InitiaizeSounds�֐�
 BGM, ���ʉ��S�̂��Ǘ�����\���̂�������
 ����
 sounds		: BGM, ���ʉ��S�̂��Ǘ�����\����
 context	: �����Đ��p�̊�{�f�[�^
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
 AddSound�֐�
 BGM, �܂��͌��ʉ���ǉ�����
 ����
 sounds	: BGM, ���ʉ��S�̂��Ǘ�����\����
 open_func		: �f�[�^���J�����߂̊֐��|�C���^
 read_func		: �f�[�^�ǂݍ��ݗp�̊֐��|�C���^
 seek_func		: �V�[�N�ʒu��ύX���邽�߂̊֐��|�C���^
 tell_func		: �V�[�N�ʒu���擾���邽�߂̊֐��|�C���^
 delete_func	: �f�[�^����邽�߂̊֐��|�C���^
 play_flags		: �����Đ��̃t���O
 user_data		: �f�[�^���J���ۂɎg�����[�U�[��`�f�[�^
 �Ԃ�l
	�����Đ����Ǘ�����f�[�^
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
	{	// �t�@�C���I�[�v�����s
		return NULL;
	}

	// �g���q���擾
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
 PlayAllSound�֐�
 �S�Ă�BGM, ���ʉ����Đ�����
 ����
 sounds	: BGM, ���ʉ��S�̂��Ǘ�����\����
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
