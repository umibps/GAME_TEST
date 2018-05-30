#ifndef _INCLUDED_TOKEN_H_
#define _INCLUDED_TOKEN_H_

#include "../types.h"

/*
 eTOKEN_TYPE�񋓑�
 �g�[�N���̎��
*/
typedef enum _eTOKEN_TYPE
{
	TOKEN_TYPE_ASSIGN,			// ���
	TOKEN_TYPE_PLUS,			// �����Z
	TOKEN_TYPE_MINUS,			// �����Z
	TOKEN_TYPE_MULTI,			// �|���Z
	TOKEN_TYPE_DIVIDE,			// ����Z
	TOKEN_TYPE_MOD,				// �]��
	TOKEN_TYPE_BOOL_AND,		// �_����
	TOKEN_TYPE_BOOL_OR,			// �_���a
	TOKEN_TYPE_BOOL_NOT,		// �_���ے�
	TOKEN_TYPE_BIT_AND,			// �r�b�g��
	TOKEN_TYPE_BIT_OR,			// �r�b�g�a
	TOKEN_TYPE_BIT_NOT,			// �r�b�g���]
	TOKEN_TYPE_BIT_XOR,			// �r�b�g�r���I�_���a
	TOKEN_TYPE_EQUAL,			// ��r(������)
	TOKEN_TYPE_NOT_EQUAL,		// ��r(�������Ȃ�)
	TOKEN_TYPE_LESS,			// ��r(����)
	TOKEN_TYPE_LESS_EQUAL,		// ��r(�ȉ�)
	TOKEN_TYPE_GREATER,			// ��r(�傫��)
	TOKEN_TYPE_GREATER_EQUAL,	// ��r(�ȏ�)
	TOKEN_TYPE_INCREMENT,		// �C���N�������g
	TOKEN_TYPE_DECREMENT,		// �f�N�������g
	TOKEN_TYPE_CONST_DIGIT,		// �����萔
	TOKEN_TYPE_CONST_FLOAT,		// �����萔
	TOKEN_TYPE_CONST_STRING,	// ������萔
	TOKEN_TYPE_SEMI_COLON,		// �Z�~�R����
	TOKEN_TYPE_COMMA,			// �R���}
	TOKEN_TYPE_LEFT_PAREN,		// �����ʎn��
	TOKEN_TYPE_RIGHT_PAREN,		// �����ʏI���
	TOKEN_TYPE_LEFT_BRACE,		// �����ʎn��
	TOKEN_TYPE_RIGHT_BRACE,		// �����ʏI���
	TOKEN_TYPE_LEFT_BRACKET,	// �劇�ʎn��
	TOKEN_TYPE_RIGHT_BRACKET,	// �劇�ʏI���
	TOKEN_TYPE_IDENT,			// �ϐ��̎��ʎq
	TOKEN_TYPE_FUNCTION,		// �֐��̎��ʎq
	TOKEN_TYPE_END_OF_CODE,		// �R�[�h�̏I�[
	NUM_DEFAULT_TOKEN_TYPE
} eTOKEN_TYPE;

/*
 TOKEN�\����
 �g�[�N��1���̏��
*/
typedef struct _TOKEN
{
	uint16 length;		// ������̒���
	uint16 token_type;	// �g�[�N���̃^�C�v
	uint16 file_id;		// �t�@�C���̎���ID
	uint16 line;		// �s��
	char *name;			// �g�[�N���̖��O
} TOKEN;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeToken�֐�
 �X�N���v�g�̎����͂ɂ���ē�����g�[�N���f�[�^������������
 ����
 token	: �g�[�N���f�[�^
 line	: �g�[�N���̑��݂���s��
*/
EXTERN void InitializeToken(TOKEN* token, int line);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TOKEN_H_
