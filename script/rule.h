#ifndef _INCLUDED_RULE_H_
#define _INCLUDED_RULE_H_

#include "token.h"
#include "abstract_syntax_tree.h"
#include "../memory_pool.h"

/*
 SCRIPT_RULE_ELEMENT�\����
 �X�N���v�g�̍\����̓��[���̊�{�f�[�^
*/
typedef struct _SCRIPT_RULE_ELEMENT
{
	// ������������郋�[��(���s��FALSE��Ԃ�)
	int (*assign_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// �����Z�܂��̓v���X�������������郋�[��(���s��FALSE��Ԃ�)
	int (*plus_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// �����Z�܂��̓}�C�i�X�������������郋�[��(���s��FALSE��Ԃ�)
	int (*minus_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// �|���Z���������郋�[��(���s��FALSE��Ԃ�)
	int (*multi_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// ����Z���������郋�[��
	int (*divide_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// �]��v�Z���������郋�[��
	int (*mod_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// �_���a���������郋�[��(���s��FALSE��Ԃ�)
	int (*bool_or_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// �_���ς��������郋�[��(���s��FALSE��Ԃ�)
	int (*bool_and_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// ���l��r���������郋�[��
	int (*equal_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// �s���l��r���������郋�[��
	int (*not_equal_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// �l��r(����)���������郋�[��
	int (*less_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// �l��r(�ȉ�)���������郋�[��
	int (*less_equal_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// �l��r(���z)���������郋�[��
	int (*greater_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// �l��r(�ȉ�)���������郋�[��
	int (*greater_equal_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// �G���[�������Ɏg�p����֐��|�C���^
	void (*error_message)(void* error_message_data, const char* file_name,
		int line, const char* message, ...);
	// �G���[���b�Z�[�W�\���Ɏg�p����f�[�^
	void *error_message_data;
	// �X�N���v�g�t�@�C���̃t�@�C����(�����t�@�C��)
	const char **file_names;
	// �������Ǘ��p�f�[�^
	MEMORY_POOL *memory_pool;
} SCRIPT_RULE_ELEMENT;

typedef struct _SCRIPT_BASIC_RULE
{
	SCRIPT_RULE_ELEMENT element;
} SCRIPT_BASIC_RULE;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeScriptBasicRule�֐�
 �f�t�H���g�̍\���`�F�b�N�����p�f�[�^��������
 ����
 rule				: �f�t�H���g�̍\���`�F�b�N�����p�f�[�^
 error_message		: �G���[���b�Z�[�W�\���p�֐�
 error_message_data	: �G���[���b�Z�[�W�\���p�֐��Ŏg�����[�U�[�f�[�^
 file_names			: �X�N���v�g�t�@�C�����̔z��
 memory_pool		: �������Ǘ��p�f�[�^
*/
EXTERN void InitializeScriptBasicRule(
	SCRIPT_BASIC_RULE* rule,
	void (*error_message)(void* error_message_data, const char* file_name, int line, const char* message, ...),
	void* error_message_data,
	const char** file_names,
	MEMORY_POOL* memory_pool
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_RULE_H_
