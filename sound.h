#ifndef _INCLUDED_SOUND_H_
#define _INCLUDED_SOUND_H_

#include <stdio.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>
#include <FLAC/stream_decoder.h>
#include "utils.h"
#include "types.h"

#define SOUND_BUFFER_SIZE 4096	// �����o�b�t�@��1�u���b�N�̃o�C�g��
#define SOUND_BUFFER_NUM 8		// �����o�b�t�@�̃u���b�N��

/*
 SOUND_CONTEXT�\����
 �����Đ��ɕK�v�ȃf�[�^
*/
typedef struct _SOUND_CONTEXT
{
	ALCdevice *device;		// �����Đ��p�̃f�o�C�X
	ALCcontext *context;	// �����Đ��p�̃R���e�L�X�g
} SOUND_CONTEXT;

typedef enum _eSOUND_PLAY_FLAGS
{
	SOUND_PLAY_NO_FLAG = 0x00,
	SOUND_PLAY_FLAG_END = 0x01,			// �Đ��I��
	SOUND_PLAY_FLAG_LOOP_PLAY = 0x02,	// ���[�v�Đ�
	SOUND_PLAY_FLAG_PAUSE = 0x04		// �ꎞ��~
} eSOUND_PLAY_FLAG;

/*
 SOUND_PLAY_BASE�\����
 �����Đ��p�̊�{�f�[�^
*/
typedef struct _SOUND_PLAY_BASE
{
	// �R���e�L�X�g
	SOUND_CONTEXT *context;
	// �����f�[�^���f�R�[�h���Ď擾����֐��|�C���^
	size_t (*get_data_func)(uint8* data, size_t size, struct _SOUND_PLAY_BASE* sound_play);
	// �Đ��ʒu��ύX����֐��|�C���^
	int (*time_seek_func)(struct _SOUND_PLAY_BASE* sound_play, FLOAT_T time_value);
	// �f�[�^�폜���Ɏ��s����֐��|�C���^
	void (*delete_func)(void* stream);
	ALuint source;						// �����\�[�X
	ALuint buffer[SOUND_BUFFER_NUM];	// �����o�b�t�@
	ALint play_state;					// �Đ����
	ALenum format;						// �����̃t�H�[�}�b�g
	ALsizei sample_rate;				// �����̃T���v�����O�E���[�g
	FLOAT_T loop_start_time;			// ���[�v�Đ��̊J�n����
	uint8 *sound_data;					// �����f�[�^�̈ꎞ�ۊǏꏊ
	int queue_size;						// �����Đ��҂��s��̃T�C�Y
	int next_unqueue;					// ���ɑ҂��s�񂩂�O���o�b�t�@
	unsigned int play_flags;			// �����Đ��֘A�̃t���O
} SOUND_PLAY_BASE;

/*
 WAVE_SOUND_PLAY�\����
 �����kWave�T�E���h���Đ����邽�߂̃f�[�^
*/
typedef struct _WAVE_SOUND_PLAY
{
	SOUND_PLAY_BASE base_data;	// �����Đ��p�̊�{�I�ȃf�[�^
	int channel;				// �`�����l����(���m����:1 �X�e���I:2)
	int bit;					// 1�T���v��������̃r�b�g��
	size_t data_size;			// �f�[�^�̃T�C�Y
	int data_top;				// �����f�[�^�̐擪�ʒu
	void *stream;				// �����f�[�^�X�g���[��
	// �f�[�^�ǂݍ��݂̊֐��|�C���^
	size_t (*read_func)(void*, size_t, size_t, void*);
	// �f�[�^�V�[�N�p�̊֐��|�C���^
	int (*seek_func)(void*, long, int);
	// �f�[�^�폜���̊֐��|�C���^
	void (*delete_func)(void*);
} WAVE_SOUND_PLAY;

/*
 VORBIS_SOUND_PLAY�\����
 Vorbis�T�E���h���Đ����邽�߂̃f�[�^
*/
typedef struct _VORBIS_SOUND_PLAY
{
	SOUND_PLAY_BASE base_data;	// �����Đ��p�̊�{�I�ȃf�[�^
	OggVorbis_File vorbis_file;	// �f�R�[�h�ɕK�v�ȃf�[�^
	vorbis_info *vorbis_info;	// 1�T���v��������̃r�b�g���A�T���v�����O���[�g���̃f�[�^
	size_t data_size;			// �f�[�^�̃T�C�Y
	int current_position;		// ���݂̍Đ��ʒu
	void *stream;				// �����f�[�^�X�g���[��
	// �f�[�^�ǂݍ��݂̊֐��|�C���^
	size_t (*read_func)(void*, size_t, size_t, void*);
	// �f�[�^�V�[�N�p�̊֐��|�C���^
	int (*seek_func)(void*, long, int);
	// �f�[�^�V�[�N�ʒu�擾�p�̊֐��|�C���^
	long (*tell_func)(void*);
	// �f�[�^�폜���̊֐��|�C���^
	void (*delete_func)(void*);
} VORBIS_SOUND_PLAY;

// �o�b�t�@�I�[�o�[�������p�ꎞ�ۊǏꏊ�̃T�C�Y
#define FLAC_CARRY_BUFFER_SIZE (4096 * 4)

/*
 FLAC_SOUND_PLAY�\����
 FLAC�T�E���h���Đ����邽�߂̃f�[�^
*/
typedef struct _FLAC_SOUND_PLAY
{
	SOUND_PLAY_BASE base_data;		// �����Đ��p�̊�{�I�ȃf�[�^
	FLAC__StreamDecoder *decoder;	// �f�R�[�h�ɕK�v�ȃf�[�^
	size_t data_size;				// �����f�[�^�̑��o�C�g��
	int write_position;				// �����o�b�t�@�ɏ������񂾃o�C�g��
	int carry_size;					// �ꎞ�ۊǏꏊ�ɒ��߂Ă���o�C�g��
	int bits_per_sample;			// 1�T���v��������̃r�b�g��
	int channels;					// �X�e���I or ���m����
	// �ꎞ�ۊǗp�̃o�b�t�@
	uint8 carry_data[FLAC_CARRY_BUFFER_SIZE];
	void *stream;				// �����f�[�^�X�g���[��
	// �f�[�^�ǂݍ��݂̊֐��|�C���^
	size_t (*read_func)(void*, size_t, size_t, void*);
	// �f�[�^�V�[�N�p�̊֐��|�C���^
	int (*seek_func)(void*, long, int);
	// �f�[�^�V�[�N�ʒu�擾�p�̊֐��|�C���^
	long (*tell_func)(void*);
	// �f�[�^�폜���̊֐��|�C���^
	void (*delete_func)(void*);
} FLAC_SOUND_PLAY;

// �����f�[�^�z��̃f�t�H���g�̃T�C�Y
#define SOUNDS_DEFAULT_BUFFER_SIZE 64

/*
 SOUND_ITEM�\����
 BGM, ���ʉ��̍Đ��A�C�e��
*/
typedef struct _SOUND_ITEM
{
	// �f�[�^�X�g���[��
	void *stream;
	// �f�[�^�ǂݍ��ݗp�֐��|�C���^
	size_t (*read_func)(void*, size_t, size_t, void*);
	// �f�[�^�V�[�N�p�֐��|�C���^
	int (*seek_func)(void*, long, int);
	// �V�[�N�ʒu�擾�p�֐��|�C���^
	long (*tell_func)(void*);
	// �f�[�^�폜�p�̊֐��|�C���^
	void (*delete_func)(void*);
	// �����Đ��p�̃f�[�^
	SOUND_PLAY_BASE *sound_play;
	// �����S�̂��Ǘ�����f�[�^
	struct _SOUNDS *sounds;
} SOUND_ITEM;

/*
 SOUNDS�\����
 BGM, ���ʉ����Ǘ�
*/
typedef struct _SOUNDS
{
	SOUND_ITEM **sounds;	// �����f�[�^�z��
	size_t num_sounds;		// ���݁A�ێ����Ă��鉹���̐�
	size_t buffer_size;		// �����f�[�^�z��̃T�C�Y
	size_t search_start;	// �����J�n�ʒu
	SOUND_CONTEXT *context;	// �����Đ��p�̊�{�f�[�^
} SOUNDS;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeSoundContext�֐�
 �����Đ��ɕK�v�Ȋ�{�I�ȃf�[�^��������
 ����
 context	: �����Đ��ɕK�v�Ȋ�{�I�ȃf�[�^
*/
EXTERN void InitializeSoundContext(SOUND_CONTEXT* context);

/*
 ReleaseSoundContext�֐�
 �����Đ��ɕK�v�Ȋ�{�I�ȃf�[�^���J��
 ����
 context	: �����Đ��ɕK�v�Ȋ�{�I�ȃf�[�^
*/
EXTERN void ReleaseSoundContext(SOUND_CONTEXT* context);

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
 ReleaseSoundPlay�֐�
 �����Đ��p�̊�{�I�ȃf�[�^���J������
 ����
 sound_play	: �����Đ��p�̊�{�I�ȃf�[�^
*/
EXTERN void ReleaseSoundPlay(SOUND_PLAY_BASE* sound_play);

/*
 PlaySound�֐�
 �����Đ������s����
 ����
 sound_play	: �����Đ��p�̊�{�I�ȃf�[�^
*/
EXTERN void PlaySound(SOUND_PLAY_BASE* sound_play);

/*
 PauseSound�֐�
 �����Đ����ꎞ��~����
 ����
 sound_play	: �����Đ��p�̊�{�I�ȃf�[�^
*/
EXTERN void PauseSound(SOUND_PLAY_BASE* sound);

/*
 StartPlaySound�֐�
 �����Đ����J�n����
 sound_play	: �����Đ��p�̊�{�I�ȃf�[�^
*/
EXTERN void StartPlaySound(SOUND_PLAY_BASE* sound);

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
 InitiaizeSounds�֐�
 BGM, ���ʉ��S�̂��Ǘ�����\���̂�������
 ����
 sounds		: BGM, ���ʉ��S�̂��Ǘ�����\����
 context	: �����Đ��p�̊�{�f�[�^
*/
EXTERN void InitializeSounds(SOUNDS* sounds, SOUND_CONTEXT* context);

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
 PlayAllSound�֐�
 �S�Ă�BGM, ���ʉ����Đ�����
 ����
 sounds	: BGM, ���ʉ��S�̂��Ǘ�����\����
*/
EXTERN void PlayAllSound(SOUNDS* sounds);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SOUND_H_
