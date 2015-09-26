#ifndef _INCLUDED_LEXICAL_ANALYSER_
#define _INCLUDED_LEXICAL_ANALYSER_

#include <stdarg.h>
#include "token.h"
#include "../memory_pool.h"
#include "../utils.h"

/*
 LEXICAL_ANALYSER�\����
 �\�[�X�R�[�h���g�[�N���ɕ�������
*/
typedef struct _LEXICAL_ANALYSER
{
	char *file_path;				// �\�[�X�R�[�h�̃t�@�C���p�X
	MEMORY_POOL memory_pool;		// �g�[�N���ێ��p�̃������v�[��
	POINTER_ARRAY file_names;		// �\�[�X�R�[�h�t�@�C��
	POINTER_ARRAY tokens;			// �g�[�N��
	// �G���[���b�Z�[�W�\���p�֐�
	void (*error_message_func)(void* data, const char* message, ... );
	void *error_message_func_data;	// �G���[�\���p�̊֐��Ŏg���f�[�^
} LEXICAL_ANALYSER;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeLexicalAnalyzer�֐�
 �\�[�X�R�[�h���g�[�N���ɕ������邽�߂̃f�[�^��������
 ����
 analyser			: �\�[�X�R�[�h���g�[�N���ɕ������邽�߂̃f�[�^
 file_path			: �\�[�X�R�[�h�̃p�X
 error_message_func	: �G���[���b�Z�[�W�\���p�֐��|�C���^
 message_func_data	: �G���[���b�Z�[�W�\���p�֐��Ŏg���f�[�^
*/
EXTERN void InitializeLexicalAnalyzer(
	LEXICAL_ANALYSER* analyser,
	const char* file_path,
	void (*error_message_func)(void*, const char*, ... ),
	void* message_func_data
);

/*
 ReleaseLexicalAnalyzer�֐�
 �\�[�X�R�[�h���g�[�N���ɕ������邽�߂̃f�[�^���J������
 ����
 analyser	: �\�[�X�R�[�h���g�[�N���ɕ������邽�߂̃f�[�^
*/
EXTERN void ReleaseLexicalAnalyzer(LEXICAL_ANALYSER* analyser);

/*
 LexicalAnayze�֐�
 �\�[�X�R�[�h���g�[�N���ɕ�������
 ����
 analyser	: �\�[�X�R�[�h���g�[�N���ɕ������邽�߂̃f�[�^
 open_func	: �\�[�X�R�[�h�f�[�^���J�����߂̊֐��|�C���^
 read_func	: �\�[�X�R�[�h�f�[�^��ǂݍ��ނ��߂̊֐��|�C���^
 seek_func	: �\�[�X�R�[�h�f�[�^���V�[�N���邽�߂̊֐��|�C���^
 tell_func	: �\�[�X�R�[�h�f�[�^�̃V�[�N�ʒu���擾���邽�߂̊֐��|�C���^
 close_func	: �\�[�X�R�[�h�f�[�^����邽�߂̊֐��|�C���^
 open_data	: �\�[�X�R�[�h���J���ۂɎg���O���f�[�^
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
EXTERN int LexicalAnalyze(
	LEXICAL_ANALYSER* analyser,
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

#endif	// #ifndef _INCLUDED_LEXICAL_ANALYSER_