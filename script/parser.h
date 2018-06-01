#ifndef _INCLUDED_PARSER_H_
#define _INCLUDED_PARSER_H_

#include "token.h"
#include "rule.h"
#include "../memory_pool.h"
#include "../utils.h"

/*
eSCRIPT_BASIC_RESERVED_TYPE�񋓑�
�f�t�H���g�̃X�N���v�g���s�����ł̗\��ꎯ��ID
*/
typedef enum _eSCRIPT_BASIC_RESERVED_TYPE
{
	SCRIPT_BASIC_RESERVED_IF = NUM_DEFAULT_TOKEN_TYPE,
	SCRIPT_BASIC_RESERVED_ELSE,
	SCRIPT_BASIC_RESERVED_WHILE,
	SCRIPT_BASIC_RESERVED_BREAK,
	SCRIPT_BASIC_RESERVED_USER_FUNCTION
} eSCRIPT_BASIC_RESERVED_TYPE;

/*
 SCRIPT_PARSER_ELEMENT�\����
 �X�N���v�g�̍\����͂̊�{�f�[�^
*/
typedef struct _SCRIPT_PARSER_ELEMENT
{
	POINTER_ARRAY abstract_syntax_tree;	// ���ۍ\����
	SCRIPT_RULE_ELEMENT *rule;			// �\�������������`�F�b�N����f�[�^
	MEMORY_POOL *memory_pool;			// ���������Ǘ�����f�[�^
	TOKEN **tokens;						// �����͂ɂ���ē���ꂽ�g�[�N��
	int num_tokens;						// �����͂ɂ���ē���ꂽ�g�[�N���̐�
	STRING_HASH_TABLE *user_functions;	// ���[�U�[��`�֐��̕�����

	// �\����͎��{�֐�(���s�����FALSE���Ԃ�)
	int (*parse)(struct _SCRIPT_PARSER_ELEMENT* parser);
	// ������\����͂���
	int (*parse_assign)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// �_���a���\����͂���
	int (*parse_bool_or)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// �_���ς��\����͂���
	int (*parse_bool_and)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// ���l��r���\����͂���
	int (*parse_equal)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// �s���l��r���\����͂���
	int (*parse_not_equal)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// �召��r(����)���\����͂���
	int (*parse_less)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// �召��r(�ȉ�)���\����͂���
	int (*parse_less_equal)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// �召��r(���z)���\����͂���
	int (*parse_greater)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// �召��r(�ȏ�)���\����͂���
	int (*parse_greater_equal)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// �|���Z���\����͂���
	int (*parse_multi)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// ����Z���\����͂���
	int (*parse_divide)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// �����Z���\����͂���
	int (*parse_plus)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// �����Z���\����͂���
	int (*parse_minus)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// ( )���\����͂���
	int (*parse_paren)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// { }���\����͂���
	int (*parse_brace)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// �֐����\����͂���
	int (*parse_function)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// ����\���y�ёg�ݍ��݊֐����\����͂���
	int (**parse_reserved)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// �������J���p�̊֐�
	void (*release)(struct _SCRIPT_PARSER_ELEMENT* parser);
} SCRIPT_PARSER_ELEMENT;

/*
 SCRIPT_BASIC_PARSER�\����
 �f�t�H���g�̃X�N���v�g�\����͊�
*/
typedef struct _SCRIPT_BASIC_PARSER
{
	SCRIPT_PARSER_ELEMENT element;
	// ���ۍ\���؂Ƀg�[�N����ǉ������痧�t���O
	unsigned int *token_check_flag;
	// �����v�Z�D��x�̉��Z�����������̐e�ύX�p
	POINTER_ARRAY new_parent;
} SCRIPT_BASIC_PARSER;

#ifdef __cplusplus
extern "C" {
#endif

/*
 ScriptParserElementSetReservedParseRule�֐�
 �\����͊�ɗ\���y�ёg�ݍ��݊֐��̍\����͗p�̃��[����ǉ�����
 ����
 parser				: �\����͊�
 parse_functions	: �\����͏������s���֐��|�C���^�z��
*/
EXTERN void ScriptParserElementSetReservedParseRule(
	SCRIPT_PARSER_ELEMENT* parser,
	int (**parse_functions)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
);

/*
 InitializeScriptBasicParser�֐�
 �f�t�H���g�̍\����͊��������
 ����
 parser				: �\����͊�
 rule				: �\����͂Ɏg�p����\���`�F�b�N�p�f�[�^
 tokens				: �����͂ɂ���ē���ꂽ�g�[�N���z��
 num_tokens			: �����͂ɂ���ē���ꂽ�g�[�N���̐�
 user_functions		: ���[�U�[��`�֐��̖��O�n�b�V���e�[�u��
*/
EXTERN void InitializeScriptBasicParser(
	SCRIPT_BASIC_PARSER* parser,
	SCRIPT_RULE_ELEMENT* rule,
	TOKEN** tokens,
	int num_tokens,
	STRING_HASH_TABLE* user_functions
);

/*
 ScriptBasicParserParseIf�֐�
 �f�t�H���g�̍\����͊��if����\�������
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
int ScriptBasicParserParseIf(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);

/*
 ScriptBasicParserParseElse�֐�
 �f�t�H���g�̍\����͊��else����\�������
 ���̊֐����Ăяo����鎞�_�ŃG���[
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	���FALSE
*/
int ScriptBasicParserParseElse(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_PARSER_H_
