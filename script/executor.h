#ifndef _INCLUDED_EXECUTOR_H_
#define _INCLUDED_EXECUTOR_H_

#include <stdio.h>
#include "../types.h"
#include "../utils.h"
#include "../memory_pool.h"
#include "operand.h"

typedef enum _eSCRIPT_BASIC_ARGUMENT_TYPE
{
	SCRIPT_BASIC_ARGUMENT_TYPEN_NONE,
	SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER,
	SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT,
	SCRIPT_BASIC_ARGUMENT_TYPE_STRING
} eSCRIPT_BASIC_ARGUMENT_TYPE;

typedef struct _SCRIPT_BASIC_ARGUMENT
{
	eSCRIPT_BASIC_ARGUMENT_TYPE type;
	union
	{
		script_int integer_value;
		script_float float_value;
		char *string_value;
	} value;
} SCRIPT_BASIC_ARGUMENT;

typedef SCRIPT_BASIC_ARGUMENT (*script_user_function)(
	struct _SCRIPT_BASIC_EXECUTOR*, void*, SCRIPT_BASIC_ARGUMENT*, int);

/*
 SCRIPT_BASIC_EXECUTOR�\����
 �X�N���v�g�̃f�t�H���g�̎��s�p�f�[�^
*/
typedef struct _SCRIPT_BASIC_EXECUTOR
{
#define SCRIPT_BASIC_EXECUTOR_STRING_BUFFER_SIZE 4096
	uint8 *code;
	size_t code_size;
	int current_position;
	int num_user_function;
	STRUCT_ARRAY register_buffer;
	STRUCT_ARRAY heap_buffer;
	STRUCT_ARRAY local_buffer;
	MEMORY_POOL memory_pool;
	char string_buffer[SCRIPT_BASIC_EXECUTOR_STRING_BUFFER_SIZE];
	script_user_function *user_function;
	void *user_data;
} SCRIPT_BASIC_EXECUTOR;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeScriptBasicExecutor�֐�
 �f�t�H���g�̃X�N���v�g���s�p�f�[�^������������
 ����
 executor			: �f�t�H���g�̃X�N���v�g���s�p�f�[�^
 code				: �o�C�g�R�[�h
 code_size			: �o�C�g�R�[�h�̃T�C�Y
 user_function		: �g�ݍ��݊֐��̊֐��|�C���^�z��
 num_user_function	: �g�ݍ��݊֐��̐�
 user_data			: �g�ݍ��݊֐��Ŏg�p����f�[�^
*/
EXTERN void InitializeScriptBasicExecutor(
	SCRIPT_BASIC_EXECUTOR* executor,
	uint8* code,
	size_t code_size,
	script_user_function* user_function,
	int num_user_function,
	void* user_data
);

/*
 ReleaseScriptBasicExecutor�֐�
 �f�t�H���g�̃X�N���v�g���s�p�f�[�^���J������
 ����
 executor	: �f�t�H���g�̃X�N���v�g���s�p�f�[�^
*/
EXTERN void ReleaseScriptBasicExecutor(SCRIPT_BASIC_EXECUTOR* executor);

/*
 ScriptBasicExecutorExecute�֐�
 �X�N���v�g�̃o�C�g�R�[�h�����s����
 ����
 executor	: �f�t�H���g�̃X�N���v�g���s�p�f�[�^
 �Ԃ�l
 �X�N���v�g�̕Ԃ�l(�f�t�H���g��0)
*/
EXTERN int ScriptBasicExecutorExecute(SCRIPT_BASIC_EXECUTOR* executor);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _INCLUDED_EXECUTOR_H_
