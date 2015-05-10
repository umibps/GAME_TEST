#ifndef _INCLUDED_FILE_ARCHIVE_H_
#define _INCLUDED_FILE_ARCHIVE_H_

#include "types.h"

/*
 FILE_ARCHIVE_ITEM�\����
 �A�[�J�C�u����1�t�@�C��
*/
typedef struct _FILE_ARCHIVE_ITEM
{
	uint32 hash_value;		// �t�@�C�����̃n�b�V���l
	char *file_name;		// �t�@�C����
	uint32 data_position;	// �t�@�C���擪�̈ʒu
	uint32 data_size;		// �t�@�C���T�C�Y
} FILE_ARCHIVE_ITEM;

/*
 FILE_ARCHIVE�\����
 �A�[�J�C�u1���̃f�[�^
*/
typedef struct _FILE_ARCHIVE
{
	FILE_ARCHIVE_ITEM *files;	// �t�@�C���̓��e
	int num_files;				// �t�@�C���̐�
	void *stream;				// �A�[�J�C�u�f�[�^�ւ̃|�C���^
	// �f�[�^�ǂݍ��ݗp�̊֐��|�C���^
	size_t (*read_func)(void*, size_t, size_t, void*);
	// �ǂݍ��݈ʒu�ړ��p�̊֐��|�C���^
	int (*seek_func)(void*, long, int);
	// �ǂݍ��݈ʒu�擾�p�̊֐��|�C���^
	long (*tell_func)(void*);
} FILE_ARCHIVE;

/*
 FILE_ARCHIVE_READ�\����
 �A�[�J�C�u���̃t�@�C���ǂݍ��ݗp�̃f�[�^
*/
typedef struct _FILE_ARCHIVE_READ
{
	FILE_ARCHIVE_ITEM item_data;	// �t�@�C����, �t�@�C���T�C�Y���̃f�[�^
	int current_position;			// ���݂̓ǂݍ��݈ʒu
	FILE_ARCHIVE *archive;			// �A�[�J�C�u�S�̂��Ǘ�����f�[�^
} FILE_ARCHIVE_READ;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeFileArchive�֐�
 �t�@�C���A�[�J�C�u�Ǘ��p�f�[�^�̏�����
 ����
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
EXTERN int InitializeFileArchive(
	FILE_ARCHIVE* archive,
	const char* archive_path,
	void* stream,
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
	long (*tell_func)(void*)
);

/*
 ReleaseFileArchive�֐�
 �t�@�C���A�[�J�C�u�Ǘ��p�f�[�^�̊J��
 ����
 archive	: �t�@�C���A�[�J�C�u�Ǘ��p�f�[�^
*/
EXTERN void ReleaseFileArchive(FILE_ARCHIVE* archive);

/*
 FileArchiveReadNew�֐�
 �t�@�C���A�[�J�C�u���̃t�@�C���ǂݍ��ݗp�f�[�^�쐬
 ����
 path		: �t�@�C����
 mode		: �_�~�[
 archive	: �A�[�J�C�u�S�̂��Ǘ�����f�[�^
 �Ԃ�l
	����I��:�f�[�^�ǂݍ��ݗp�̃f�[�^	�ُ�I��:NULL
*/
EXTERN FILE_ARCHIVE_READ* FileArchiveReadNew(const char* path, const char* mode, FILE_ARCHIVE* archive);

/*
 DeleteFileArchiveRead
 �t�@�C���A�[�J�C�u���̃f�[�^�ǂݍ��ݗp�f�[�^���폜
 ����
 archive_item	: �f�[�^�ǂݍ��ݗp�̃f�[�^
 �Ԃ�l
	���0
*/
EXTERN int DeleteFileArchiveRead(FILE_ARCHIVE_READ* archive_item);

/*
 FileArchiveRead�֐�
 �t�@�C���A�[�J�C�u���̃f�[�^��ǂݍ���
 ����
 buffer			: �f�[�^���i�[����o�b�t�@
 block_size		: �f�[�^1���̃o�C�g��
 num_blocks		: �ǂݍ��ރf�[�^��
 archive_item	: �f�[�^�ǂݍ��ݗp�̃f�[�^
 �Ԃ�l
	�ǂݍ��񂾃f�[�^��
*/
EXTERN size_t FileArchiveRead(void* buffer, size_t block_size, size_t num_blocks, FILE_ARCHIVE_READ* archive_item);

/*
 FileArchiveSeek�֐�
 �t�@�C���A�[�J�C�u�ǂݍ��݈ʒu��ύX����
 ����
 archive_item	: �f�[�^�ǂݍ��ݗp�̃f�[�^
 offset			: �V�[�N�J�n�ʒu����̃I�t�Z�b�g
 whence			: �V�[�N�J�n�ʒu(fseek�Ɠ���)
 �Ԃ�l
	����I��:0	�ُ�I��:-1
*/
EXTERN int FileArchiveSeek(FILE_ARCHIVE_READ* archive_item, long offset, int whence);

/*
 FileArchiveTell�֐�
 �t�@�C���A�[�J�C�u�ǂݍ��݈ʒu���擾����
 ����
 archive_item	: �f�[�^�ǂݍ��ݗp�̃f�[�^
 �Ԃ�l
	�ǂݍ��݈ʒu
*/
EXTERN long FileArchiveTell(FILE_ARCHIVE_READ* archive_item);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_FILE_ARCHIVE_H_
