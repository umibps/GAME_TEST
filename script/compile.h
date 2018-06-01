#ifndef _INCLUCED_COMPILE_H_
#define _INCLUCED_COMPILE_H_

#include <stdarg.h>
#include "lexical_analyser.h"
#include "rule.h"
#include "parser.h"
#include "../utils.h"
#include "../memory_pool.h"

/*
 SCRIPT_BASIC_COMPILER�\����
 �f�t�H���g�̃X�N���v�g�R���p�C���p�f�[�^
*/
typedef struct _SCRIPT_BASIC_COMPILER
{
	LEXICAL_ANALYSER analyser;
	SCRIPT_BASIC_RULE rule;
	SCRIPT_BASIC_PARSER parser;
	STRING_HASH_TABLE user_function_names;
	void *user_data;
	MEMORY_POOL memory_pool;
} SCRIPT_BASIC_COMPILER;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeScriptBasicCompiler�֐�
 �f�t�H���g�̃X�N���v�g�̃R���p�C��������������
 ����
 compiler				: �R���p�C���̃f�[�^
 file_path				: ���s����X�N���v�g�̃t�@�C���p�X
 user_function_names	: ���[�U�[��`�֐��̖��O�z��
 num_user_functions		: ���[�U�[��`�֐��̐�
 user_data				: ���[�U�[��`�֐��Ŏg�����[�U�[�f�[�^
 error_message			: �G���[���b�Z�[�W�\���p�֐�
 error_message_data		: �G���[���b�Z�[�W�\���p�֐��Ŏg�����[�U�[�f�[�^
*/
EXTERN void InitializeScriptBasicCompiler(
	SCRIPT_BASIC_COMPILER* compiler,
	const char* file_path,
	const char** user_function_names,
	int num_user_functions,
	void* user_data,
	void (*error_message)(void*, const char*, ...),
	void* error_message_data
);

/*
 ReleaseScriptBasiccompiler�֐�
 �f�t�H���g�̃X�N���v�g�̃R���p�C���f�[�^�̃������J��
 ����
 compiler	: �f�t�H���g�̃X�N���v�g�̃R���p�C���f�[�^
*/
EXTERN void ReleaseScriptBasicCompiler(SCRIPT_BASIC_COMPILER* compiler);

/*
 ScriptBasicCompilerCompile�֐�
 �f�t�H���g�̃X�N���v�g�R���p�C���ŃR���p�C�����s
 ����
 compiler	: �f�t�H���g�̃X�N���v�g�̃R���p�C���f�[�^
 open_func	: �\�[�X�R�[�h�f�[�^���J�����߂̊֐��|�C���^
 read_func	: �\�[�X�R�[�h�f�[�^��ǂݍ��ނ��߂̊֐��|�C���^
 seek_func	: �\�[�X�R�[�h�f�[�^���V�[�N���邽�߂̊֐��|�C���^
 tell_func	: �\�[�X�R�[�h�f�[�^�̃V�[�N�ʒu���擾���邽�߂̊֐��|�C���^
 close_func	: �\�[�X�R�[�h�f�[�^����邽�߂̊֐��|�C���^
 open_data	: �\�[�X�R�[�h���J���ۂɎg���O���f�[�^
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
EXTERN int ScriptBasicCompilerCompile(
	SCRIPT_BASIC_COMPILER* compiler,
	void* (*open_func)(const char*, const char*, void*),
	size_t (*read_func)(void*, size_t, size_t, void*),
	void* (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	int (*close_func)(void*),
	void* open_data
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUCED_COMPILE_H_
