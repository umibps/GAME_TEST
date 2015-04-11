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

	if(sound_play->delete_func != NULL)
	{
		sound_play->delete_func(sound_play);
	}

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

	// �R���e�L�X�g���Z�b�g
	alcMakeContextCurrent(sound_play->context->context);

	// �����\�[�X�̏�Ԃ��擾
	alGetSourcei(sound_play->source, AL_SOURCE_STATE, &sound_play->play_state);

	// �Đ���ԂłȂ���΍Đ��J�n
	if(sound_play->play_state != AL_PLAYING && sound_play->play_state != AL_STOPPED)
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
					continue;
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

#ifdef __cplusplus
}
#endif
