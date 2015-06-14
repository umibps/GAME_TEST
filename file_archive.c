#include <stdio.h>
#include <string.h>
#include "file_archive.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeFileArchive�֐�
 �t�@�C���A�[�J�C�u�Ǘ��p�f�[�^�̏�����
 ����
 archive        : �A�[�J�C�u���Ǘ�����\���̂̃A�h���X
 archive_path   : �A�[�J�C�u�E�t�@�C���̃p�X
 stream         : �A�[�J�C�u��ǂݍ��ވׂ̃f�[�^
 read_func      : �f�[�^�ǂݍ��݂Ɏg���֐��|�C���^
 seek_func      : �f�[�^�̃V�[�N�Ɏg���֐��|�C���^
 tell_func      : �f�[�^�̃V�[�N�ʒu�擾�Ɏg���֐��|�C���^
 close_func     : �A�[�J�C�u�����ۂɎg���֐��|�C���^
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
int InitializeFileArchive(
	FILE_ARCHIVE* archive,
	const char* archive_path,
	void* stream,
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
    long (*tell_func)(void*),
    int (*close_func)(void*)
)
{
	uint8 data[8];
	uint32 data32;
	int i;

	(void)memset(archive, 0, sizeof(*archive));

	if(stream == NULL)
	{
		return FALSE;
	}

	archive->stream = stream;
	archive->read_func = read_func;
	archive->seek_func = seek_func;
	archive->tell_func = tell_func;
    archive->close_func = close_func;

	// �t�@�C���̐���ǂݍ���
	(void)read_func(data, 1, 4, stream);
	archive->num_files = (data[0] << 24) | (data[1] << 16)
		| (data[2] << 8) | data[3];

	// �w�b�_����ǂݍ��ޗ̈���m��
	archive->files = (FILE_ARCHIVE_ITEM*)MEM_ALLOC_FUNC(sizeof(*archive->files)*archive->num_files);

	// �w�b�_����ǂݍ���
	for(i=0; i<archive->num_files; i++)
	{
		// �t�@�C�����̃n�b�V���l
		(void)read_func(data, 1, 4, stream);
		archive->files[i].hash_value = (data[0] << 24) | (data[1] << 16)
			| (data[2] << 8) | data[3];

		// �t�@�C����
		(void)read_func(data, 1, 4, stream);
		data32 = (data[0] << 24) | (data[1] << 16)
			| (data[2] << 8) | data[3];
		archive->files[i].file_name = (char*)MEM_ALLOC_FUNC(data32);
		(void)read_func(archive->files[i].file_name, 1, data32, stream);

		// �t�@�C���̈ʒu
		(void)read_func(data, 1, 4, stream);
		archive->files[i].data_position = (data[0] << 24) | (data[1] << 16)
			| (data[2] << 8) | data[3];

		// �t�@�C���T�C�Y
		(void)read_func(data, 1, 4, stream);
		archive->files[i].data_size = (data[0] << 24) | (data[1] << 16)
			| (data[2] << 8) | data[3];
	}

	return TRUE;
}

/*
 ReleaseFileArchive�֐�
 �t�@�C���A�[�J�C�u�Ǘ��p�f�[�^�̊J��
 ����
 archive	: �t�@�C���A�[�J�C�u�Ǘ��p�f�[�^
*/
void ReleaseFileArchive(FILE_ARCHIVE* archive)
{
	int i;

	for(i=0; i<archive->num_files; i++)
	{
		MEM_FREE_FUNC(archive->files[i].file_name);
	}
	MEM_FREE_FUNC(archive->files);

	archive->num_files = 0;

	if(archive->close_func != NULL)
	{
		(void)archive->close_func(archive->stream);
	}
}

static int CompareFileArchiveItem(const FILE_ARCHIVE_ITEM* item1, const FILE_ARCHIVE_ITEM* item2)
{
    if(item2->hash_value > item1->hash_value)
    {
        return -1;
    }
	return item1->hash_value - item2->hash_value;
}

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
FILE_ARCHIVE_READ* FileArchiveReadNew(const char* path, const char* mode, FILE_ARCHIVE* archive)
{
	FILE_ARCHIVE_READ *ret;
	FILE_ARCHIVE_ITEM item;
	int id;

	item.hash_value = GetStringHashValue(path);
	item.file_name = (char*)path;

	id = BinarySearch(archive->files, &item, sizeof(item),
		archive->num_files, (int(*)(void*, void*, void*))CompareFileArchiveItem, NULL);
	if(id < 0)
	{
		return NULL;
	}

	ret = (FILE_ARCHIVE_READ*)MEM_ALLOC_FUNC(sizeof(*ret));

	ret->item_data = archive->files[id];
	ret->current_position = 0;
    ret->archive = archive;

	return ret;
}

/*
 DeleteFileArchiveRead
 �t�@�C���A�[�J�C�u���̃f�[�^�ǂݍ��ݗp�f�[�^���폜
 ����
 archive_item	: �f�[�^�ǂݍ��ݗp�̃f�[�^
 �Ԃ�l
	���0
*/
int DeleteFileArchiveRead(FILE_ARCHIVE_READ* archive_item)
{
	MEM_FREE_FUNC(archive_item);

	return 0;
}

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
size_t FileArchiveRead(void* buffer, size_t block_size, size_t num_blocks, FILE_ARCHIVE_READ* archive_item)
{
	FILE_ARCHIVE *archive = archive_item->archive;
	size_t num_read;

	// �ǂݍ��ݑO�ɃV�[�N���Ă���
	(void)archive->seek_func(archive->stream,
		archive_item->item_data.data_position + archive_item->current_position, SEEK_SET);

	// �ǂݍ��ރo�C�g�����v�Z
	num_read = (archive_item->current_position + block_size * num_blocks <= archive_item->item_data.data_size)
		? num_blocks : (archive_item->item_data.data_size - archive_item->current_position) / block_size;

	// �f�[�^�ǂݍ��݈ʒu���X�V
	archive_item->current_position += (int)(num_read * block_size);

	return archive->read_func(buffer, block_size, num_read, archive->stream);
}

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
int FileArchiveSeek(FILE_ARCHIVE_READ* archive_item, long offset, int whence)
{
	int new_position = offset;

	switch(whence)
	{
	case SEEK_SET:
		break;
	case SEEK_CUR:
		new_position += archive_item->current_position;
		break;
	case SEEK_END:
		new_position = archive_item->item_data.data_size + offset;
		break;
	default:
		return -1;
	}

	if(new_position < 0)
	{
		new_position = 0;
	}
	else if(new_position > (int)archive_item->item_data.data_size)
	{
		new_position = archive_item->item_data.data_size;
	}

	archive_item->current_position = new_position;

	return 0;
}

/*
 FileArchiveTell�֐�
 �t�@�C���A�[�J�C�u�ǂݍ��݈ʒu���擾����
 ����
 archive_item	: �f�[�^�ǂݍ��ݗp�̃f�[�^
 �Ԃ�l
	�ǂݍ��݈ʒu
*/
long FileArchiveTell(FILE_ARCHIVE_READ* archive_item)
{
	return (long)archive_item->current_position;
}

#ifdef __cplusplus
}
#endif
