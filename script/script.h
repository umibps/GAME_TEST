#ifndef _INCLUDED_SCRIPT_H_
#define _INCLUDED_SCRIPT_H_

#include "rule.h"
#include "parser.h"
#include "lexical_analyser.h"

/*
 SCRIPT_BASIC_EXECUTOR�\����
 �f�t�H���g�̃X�N���v�g���s�p�f�[�^
*/
typedef struct _SCRIPT_BASIC_EXECUTOR
{
	LEXICAL_ANALYSER analyser;
	SCRIPT_BASIC_RULE rule;
	SCRIPT_BASIC_PARSER parser;
	const char **user_function_names;
	int num_user_functions;
	void *user_data;
	MEMORY_POOL memory_pool;
} SCRIPT_BASIC_EXECUTOR;

#ifdef __cplusplus
extern "C" {
#endif

/*
 DefaultScriptErrorMessage�֐�
 �f�t�H���g�̃X�N���v�g�������̃G���[�\���֐�
 ����
 dummy
 file_name	: �G���[�̂������t�@�C����
 line		: �G���[�̔��������s�� (0�̏ꍇ�̓X�N���v�g�̓��e�ȊO�̃G���[)
 message	: �G���[���b�Z�[�W
 ... : printf�Ɠ���
*/
EXTERN void DefaultScriptErrorMessage(
	void* dummy,
	const char* file_name,
	int line,
	const char* message,
	...
);

/*
 InitializreScriptBasicExecutor�֐�
 �f�t�H���g�̃X�N���v�g���s�p�f�[�^��������
 ����
 executor				: �f�t�H���g�̃X�N���v�g���s�p�f�[�^
 file_path				: ���s����X�N���v�g�̃t�@�C���p�X
 user_function_names	: ���[�U�[��`�֐��̖��O�z��
 num_user_functions		: ���[�U�[��`�֐��̐�
 user_data				: ���[�U�[��`�֐��Ŏg�����[�U�[�f�[�^
 error_message			: �G���[���b�Z�[�W�\���p�֐�
 error_message_data		: �G���[���b�Z�[�W�\���p�֐��Ŏg�����[�U�[�f�[�^
*/
EXTERN void InitializeScriptBasicExecutor(
	SCRIPT_BASIC_EXECUTOR* executor,
	const char* file_path,
	const char** user_function_names,
	int num_user_functions,
	void* user_data,
	void (*error_message)(void*, const char*, ...),
	void* error_message_data
);

/*
 ReleaseScriptBasicExecutor�֐�
 �f�t�H���g�̃X�N���v�g���s�p�f�[�^�̃������J��
 ����
 executor	: �f�t�H���g�̃X�N���v�g���s�p�f�[�^
*/
EXTERN void ReleaseScriptBasicExecutor(SCRIPT_BASIC_EXECUTOR* executor);

/*
 ScriptBasicEcecutorExecute�֐�
 �f�t�H���g�̃X�N���v�g���s�p�f�[�^�ŃX�N���v�g���s
 ����
 executor	: �f�t�H���g�̃X�N���v�g���s�p�f�[�^
 open_func	: �\�[�X�R�[�h�f�[�^���J�����߂̊֐��|�C���^
 read_func	: �\�[�X�R�[�h�f�[�^��ǂݍ��ނ��߂̊֐��|�C���^
 seek_func	: �\�[�X�R�[�h�f�[�^���V�[�N���邽�߂̊֐��|�C���^
 tell_func	: �\�[�X�R�[�h�f�[�^�̃V�[�N�ʒu���擾���邽�߂̊֐��|�C���^
 close_func	: �\�[�X�R�[�h�f�[�^����邽�߂̊֐��|�C���^
 open_data	: �\�[�X�R�[�h���J���ۂɎg���O���f�[�^
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
EXTERN int ScriptBasicExecutorExecute(
	SCRIPT_BASIC_EXECUTOR* executor,
	void* (*open_func)(const char*, const char*, void*),
	size_t (*read_func)(void*, size_t, size_t, void*),
	void* (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	int (*close_func)(void*),
	void* open_data
);

/*
 ReleaseScriptBasicExecutor�֐�
 �f�t�H���g�̃X�N���v�g���s�p�f�[�^�̃������J��
 ����
 executor	: �f�t�H���g�̃X�N���v�g���s�p�f�[�^
*/
EXTERN void ReleaseScriptBasicExecutor(SCRIPT_BASIC_EXECUTOR* executor);

/*
 ScriptBasicGetDefaultReservedStrings�֐�
 �f�t�H���g�̗\���̕�����z����擾����
 ����
 num_strings	: �\���̐�
 �Ԃ�l
	�f�t�H���g�̗\���̕�����z��
*/
EXTERN const char** ScriptBasicGetDefaultReservedStrings(int* num_strings);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SCRIPT_H_
