#include <stdio.h>
#include <ctype.h>
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 StringCompareIgnoreCase�֐�
 �啶��/�������𖳎����ĕ�������r
 ����
 str1	: ��r���镶���񂻂�1
 str2	: ��r���镶���񂻂�2
 �Ԃ�l
	str1 - str2
*/
int StringCompareIgnoreCase(const char* str1, const char* str2)
{
	int ret;

	while((ret = toupper(*str1) - toupper(*str2)) == 0)
	{
		str1++, str2++;
		if(*str1 == '\0')
		{
			return 0;
		}
	}

	return ret;
}

/*
 GetNextUtf8Character�֐�
 ����UTF8�̕������擾����
 ����
 str	: ���̕������擾������������
 �Ԃ�l
	���̕����̃A�h���X
*/
const char* GetNextUtf8Character(const char* str)
{
	static const unsigned char skips[256] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
	};

	return str + skips[((unsigned char)*str)];
}

/*
 FileOpen�֐�
 �t�@�C�����J��
 ����
 path		: �t�@�C���̃p�X
 mode		: �J�����[�h(fopen�֐��ɏ���)
 user_data	: �v���O���}�[�����R�Ɏg����f�[�^
 �Ԃ�l
	����:�t�@�C���|�C���^	���s:NULL
*/
FILE* FileOpen(const char* path, const char* mode, void* user_data)
{
	return fopen(path, mode);
}

/*
 FileClose�֐�
 �t�@�C�������
 ����
 fp			: �t�@�C���|�C���^
 user_data	: �v���O���}�[�����R�Ɏg����f�[�^
 �Ԃ�l
	���0
*/
int FileClose(FILE* fp, void* user_data)
{
	(void)fclose(fp);

	return 0;
}

/*
 BinarySearch�֐�
 �񕪒T�������s����
 ����
 search_array	: �T�������z��
 target			: �T�����s���f�[�^
 block_size		: �z���1���̗v�f�̃o�C�g��
 num_blocks		: �z��̃T�C�Y
 compare_func	: �f�[�^��r�p�̊֐�
					(���l:0 ��Ȃ�:1 ���Ȃ�:-1��Ԃ�)
 user_data		: compare_func�̑�3�����ɓ��郆�[�U�[�f�[�^
 �Ԃ�l
	���������v�f��ID (������Ȃ��ꍇ�͍ł��߂����̕���)
*/
int BinarySearch(
	void* search_array,
	void* target,
	int block_size,
	int num_blocks,
	int(*compare_func)(void*, void*, void*),
	void* user_data
)
{
	unsigned char *bytes = (unsigned char*)search_array;
	int compare;
	int minimum = 0;
	int maximum = num_blocks - 1;
	int midium;

	while(minimum <= maximum)
	{
		midium = (minimum + maximum) / 2;
		compare = compare_func((void*)&bytes[midium*block_size], target, user_data);
		if(compare == 0)
		{
			while(midium < num_blocks-1
				&& compare_func(&bytes[(midium+1)*block_size], target, user_data) == 0)
			{
				midium++;
			}
			return midium;
		}
		else if(compare < 0)
		{
			minimum = midium + 1;
		}
		else
		{
			maximum = midium - 1;
		}
	}

	if(maximum < 0)
	{
		maximum = 0;
	}
	while(maximum > 0
		&& compare_func(&bytes[(maximum-1)*block_size],
		&bytes[maximum*block_size], user_data) > 0)
	{
		maximum--;
	}
	while(maximum < num_blocks
		&& compare_func(&bytes[(maximum+1)*block_size],
		&bytes[maximum*block_size], user_data) < 0)
	{
		maximum++;
	}

	return - maximum;
}

/*
 InitializePriorityArray�֐�
 �D��x�t���z�������������
 ����
 priority_array	: ����������D��x�t���z��
 block_size		: �o�b�t�@�T�C�Y�g�厞�̍X�V��
 delete_func	: �f�[�^�폜���Ɏg���֐��|�C���^
*/
void InitializePriorityArray(
	PRIORITY_ARRAY* priority_array,
	size_t block_size,
	void (*delete_func)(void*)
)
{
	(void)memset(priority_array, 0, sizeof(priority_array));

	priority_array->block_size = priority_array->buffer_size
		= block_size;
	priority_array->buffer =
		(PRIORITY_ARRAY_ITEM*)MEM_CALLOC_FUNC(block_size, sizeof(*priority_array->buffer));
	priority_array->delete_func = delete_func;
}

/*
 ReleasePriorityArray�֐�
 �D��x�t���z����J������
 ����
 priority_array	: �D��x�t���z��
*/
void ReleasePriorityArray(PRIORITY_ARRAY* priority_array)
{
	if(priority_array->delete_func != NULL)
	{
		unsigned int i;
		for(i=0; i<(unsigned int)priority_array->num_data; i++)
		{
			priority_array->delete_func(priority_array->buffer[i].data);
		}
	}
	MEM_FREE_FUNC(priority_array->buffer);
}

/*
 ComparePriorityArrayItem�֐�
 �񕪒T���p�̔�r�֐�
 ����
 item1	: ��r�Ώ�1
 item2	: ��r�Ώ�2
 dummy	: �ėp�|�C���^(�g�p���Ȃ�)
 �Ԃ�l
	�D��x����:-1	�D��x����:0	�D��x�Ⴂ:1�ȏ�
*/
static int ComparePriorityArrayItem(
	PRIORITY_ARRAY_ITEM* item1,
	PRIORITY_ARRAY_ITEM* item2,
	void* dummy
)
{
	if(item1->priority < item2->priority)
	{
		return -1;
	}

	return item1->priority - item2->priority;
}

/*
 PriorityArrayAppend�֐�
 �D��x�t���z��Ƀf�[�^��1�ǉ�����
 ����
 priority_array	: �D��x�t���z��
 data			: �ǉ�����f�[�^�̃A�h���X
 priority		: �ǉ�����f�[�^�̗D��x
*/
void PriorityArrayAppend(
	PRIORITY_ARRAY* priority_array,
	void* data,
	unsigned int priority
)
{
	PRIORITY_ARRAY_ITEM item = {data, priority};

	if(priority_array->num_data == 0)
	{
		priority_array->buffer[0] = item;
	}
	else
	{
		int place = BinarySearch(priority_array->buffer, &item,
			sizeof(item), priority_array->num_data,
			(int(*)(void*, void*, void*))ComparePriorityArrayItem, NULL);
		unsigned int i;
		if(place >= 0)
		{
			place++;
		}
		else
		{
			place = -place;
		}
		for(i=priority_array->num_data; i>(unsigned int)place; i--)
		{
			priority_array->buffer[i] = priority_array->buffer[i-1];
		}
		priority_array->buffer[place] = item;
	}
	priority_array->num_data++;

	if(priority_array->num_data == priority_array->buffer_size)
	{
		size_t before_size = priority_array->buffer_size;
		priority_array->buffer_size += priority_array->block_size;
		priority_array->buffer = (PRIORITY_ARRAY_ITEM*)MEM_REALLOC_FUNC(
			priority_array->buffer, sizeof(*priority_array->buffer)*priority_array->buffer_size);
		(void)memset(&priority_array->buffer[before_size], 0,
			sizeof(*priority_array->buffer)*priority_array->block_size);
	}
}

/*
 PriorityArrayRemove�֐�
 �D��x�t���z�񂩂�f�[�^��1��菜��
 ����
 priority_array	: �D��x�t���z��
 data			: ��菜���f�[�^
*/
void PriorityArrayRemove(
	PRIORITY_ARRAY* priority_array,
	void* data
)
{
	unsigned int i;
	for(i=0; i<(unsigned int)priority_array->num_data; i++)
	{
		if(priority_array->buffer[i].data == data)
		{
			unsigned int j;
			for(j=priority_array->num_data-1; j>i; j++)
			{
				priority_array->buffer[i-1] = priority_array->buffer[i];
			}
			if(priority_array->delete_func != NULL)
			{
				priority_array->delete_func(data);
			}
			priority_array->num_data--;
			return;
		}
	}
}

/*
 PriorityArrayReset�֐�
 �D��x�t���z��̓��e�����Z�b�g����
 ����
 priority_array	: �D��x�t���z��
*/
void PriorityArrayReset(PRIORITY_ARRAY* priority_array)
{
	unsigned int i;

	if(priority_array->delete_func != NULL)
	{
		for(i=0; i<(unsigned int)priority_array->num_data; i++)
		{
			priority_array->delete_func(priority_array->buffer[i].data);
		}
	}

	(void)memset(priority_array->buffer, 0,
		sizeof(*priority_array->buffer)*priority_array->buffer_size);

	priority_array->num_data = 0;
}

/*
 InitializeStringHashTable�֐�
 ��������L�[�Ƃ���n�b�V���e�[�u��������������
 ����
 hash_table		: �n�b�V���e�[�u�����Ǘ�����f�[�^
 block_size		: �o�b�t�@�T�C�Y���X�V����ۂ̍X�V��
 delete_func	: �f�[�^�폜���Ɏg���֐��|�C���^
*/
void InitializeStringHashTable(
	STRING_HASH_TABLE* hash_table,
	size_t block_size,
	void (*delete_func)(void*)
)
{
	(void)memset(hash_table, 0, sizeof(hash_table));

	hash_table->block_size = block_size;
	hash_table->buffer =
		(STRING_HASH_TABLE_ITEM*)MEM_CALLOC_FUNC(block_size, sizeof(*hash_table->buffer));
	hash_table->delete_func = delete_func;
}

/*
 ReleaseStringHashTable�֐�
 ��������L�[�Ƃ���n�b�V���e�[�u�����J������
 ����
 hash_table	: �n�b�V���e�[�u�����Ǘ�����f�[�^
*/
void ReleaseStringHashTable(STRING_HASH_TABLE* hash_table)
{
	unsigned int i;

	if(hash_table->delete_func == NULL)
	{
		for(i=0; i<hash_table->num_data; i++)
		{
			MEM_FREE_FUNC((void*)hash_table->buffer[i].key);
		}
	}
	else
	{
		for(i=0; i<hash_table->num_data; i++)
		{
			hash_table->delete_func(hash_table->buffer[i].data);
			MEM_FREE_FUNC((void*)hash_table->buffer[i].key);
		}
	}
}

/*
 CompareStringHashTableItem�֐�
 �񕪒T���p�̔�r�֐�
 ����
 item1	: ��r�Ώ�1
 item2	: ��r�Ώ�2
 dummy	: �ėp�|�C���^(�g�p���Ȃ�)
 �Ԃ�l
	�n�b�V���l������:-1	�n�b�V���l���l:0	�n�b�V���l�傫��:1
*/
static int CompareStringHashTableItem(
	STRING_HASH_TABLE_ITEM* item1,
	STRING_HASH_TABLE_ITEM* item2,
	void* dummy
)
{
	if(item1->hash_value == item2->hash_value)
	{
		return strcmp(item1->key, item2->key);
	}
	else if(item1->hash_value < item2->hash_value)
	{
		return -1;
	}

	return 1;
}

/*
 GetStringHashValue�֐�
 ������̃n�b�V���l���擾����
 ����
 key	: �n�b�V���l���擾���镶����
 �Ԃ�l
	�n�b�V���l
*/
unsigned int GetStringHashValue(const char* key)
{
#define INITIAL_FNV (2166136261u)
#define FNV_MULTIPLE (16777619u)
	unsigned int hash_value = INITIAL_FNV;
	const char *str = key;

	while(*str != '\0')
	{
		hash_value = hash_value ^ (*str);
		hash_value *= FNV_MULTIPLE;
		str++;
	}

	return hash_value;
}

/*
 StringHashTableGet�֐�
 �n�b�V���e�[�u������l���擾����
 ����
 hash_table	: �n�b�V���e�[�u�����Ǘ�����f�[�^
 key		: ��������f�[�^�̃L�[
 �Ԃ�l
	�L�[�ɑΉ�����f�[�^ (������Ȃ��ꍇ��NULL)
*/
void* StringHashTableGet(
	STRING_HASH_TABLE* hash_table,
	const char* key
)
{
	STRING_HASH_TABLE_ITEM item;
	int place;

	item.hash_value = GetStringHashValue(key);
	item.key = key;

	if(hash_table->num_data == 0)
	{
		return NULL;
	}

	place = BinarySearch(hash_table->buffer, &item,
		sizeof(item), (int)hash_table->num_data,
		(int(*)(void*, void*, void*))CompareStringHashTableItem, NULL
	);

	if(place < 0)
	{
		return NULL;
	}
	else if(place == 0 && item.hash_value != hash_table->buffer[place].hash_value)
	{
		return NULL;
	}

	return hash_table->buffer[place].data;
}

/*
 StringHashTableAppend�֐�
 �n�b�V���e�[�u���ɒl��ǉ�����
 ����
 hash_table	: �n�b�V���e�[�u�����Ǘ�����f�[�^
 key		: �f�[�^�ɕt�^����L�[�̕�����
 data		: �ǉ�����f�[�^
 �Ԃ�l
	�ǉ������f�[�^��ID
*/
int StringHashTableAppend(
	STRING_HASH_TABLE* hash_table,
	const char* key,
	void* data
)
{
	STRING_HASH_TABLE_ITEM item;
	int place;

	item.hash_value = GetStringHashValue(key);
	item.key = key;
	item.data = data;

	if(hash_table->num_data == 0)
	{
		item.key = MEM_STRDUP_FUNC(key);
		hash_table->buffer[0] = item;
		hash_table->num_data++;
		return 0;
	}
	place = BinarySearch(hash_table->buffer, &item,
		sizeof(item), (int)hash_table->num_data,
		(int (*)(void*, void*, void*))CompareStringHashTableItem, NULL
	);

	if(place < 0)
	{
		unsigned int i;

		place = - place;

		for(i=hash_table->num_data; i>(unsigned int)place; i--)
		{
			hash_table->buffer[i] = hash_table->buffer[i-1];
		}
		hash_table->buffer[place] = item;

		hash_table->num_data++;

		if(hash_table->num_data == hash_table->buffer_size)
		{
			size_t before_size = hash_table->buffer_size;
			hash_table->buffer_size += hash_table->block_size;
			hash_table->buffer = (STRING_HASH_TABLE_ITEM*)MEM_REALLOC_FUNC(
				hash_table->buffer, sizeof(*hash_table->buffer)*hash_table->buffer_size);
			(void)memset(&hash_table->buffer[before_size], 0,
				sizeof(*hash_table->buffer)*hash_table->block_size);
		}

		return place;
	}

	return place;
}

/*
 StringHashTableRemove�֐�
 ��������L�[�Ƃ���n�b�V���e�[�u������1�f�[�^���폜����
 ����
 hash_table	: �n�b�V���e�[�u�����Ǘ�����f�[�^
 key		: �폜����f�[�^�̃L�[
*/
void StringHashTableRemove(
	STRING_HASH_TABLE* hash_table,
	const char* key
)
{
	STRING_HASH_TABLE_ITEM item;
	int place;

	item.hash_value = GetStringHashValue(key);
	item.key = key;

	if(hash_table->num_data == 0)
	{
		return;
	}

	place = BinarySearch(hash_table->buffer, &item,
		sizeof(item), (int)hash_table->num_data,
		(int (*)(void*, void*, void*))CompareStringHashTableItem, NULL
	);
	if(place >= 0)
	{
		unsigned int i;

		item.data = hash_table->buffer[place].data;
		MEM_FREE_FUNC((void*)hash_table->buffer[place].key);

		for(i=hash_table->num_data-1; i>=(unsigned int)place; i++)
		{
			hash_table->buffer[i] = hash_table->buffer[i-1];
		}
		hash_table->num_data--;

		if(hash_table->delete_func != NULL)
		{
			hash_table->delete_func(item.data);
		}
	}
}

#ifdef __cplusplus
}
#endif
