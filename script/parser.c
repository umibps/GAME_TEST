#include <string.h>
#include "parser.h"

#define SCRIPT_PARSER_MEMORY_POOL_SIZE 4096

#ifdef __cplusplus
extern "C" {
#endif

/*
 ScriptParserElementNewLeafBinary�֐�
 2�����Z�ɑ΂��Ē��ۍ\���؂�V���Ɋm�ۂ���
 ����
 parser	: �\����͊�
 parent	: �e�m�[�h
 left	: �����̗t
 right	: �E���̗t
*/
void ScriptParserElementNewLeafBinary(SCRIPT_PARSER_ELEMENT* parser, ABSTRACT_SYNTAX_TREE* parent, TOKEN* left, TOKEN* right)
{
	parent->left = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->memory_pool, sizeof(*parent->left));
	parent->left->token = left;
	parent->left->left = parent->left->center = parent->left->right = NULL;
	parent->right = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->memory_pool, sizeof(*parent->right));
	parent->right->token = right;
	parent->right->left = parent->right->center = parent->right->right = NULL;
}

/*
 GetLeftTokenID�֐�
 �w�肳�ꂽ�g�[�N���̍����̃g�[�N�����擾����
 �����ʂ͓ǂݔ�΂�
 ����
 tokens		: �����͂ɂ���ē���ꂽ�g�[�N��
 token_id	: �w�肷��g�[�N����ID
 �Ԃ�l
	�w�肳�ꂽ�g�[�N���̍����̃g�[�N����ID
*/
static int GetLeftTokenID(TOKEN** tokens, int token_id)
{
	if(tokens[token_id]->token_type == TOKEN_TYPE_RIGHT_PAREN)
	{
		int id = token_id - 1;
		int hierarchy = 1;
		while(id > 0 && hierarchy > 0)
		{
			if(tokens[id]->token_type == TOKEN_TYPE_LEFT_PAREN)
			{
				hierarchy--;
			}
			else if(tokens[id]->token_type == TOKEN_TYPE_RIGHT_PAREN)
			{
				hierarchy++;
			}
			id--;
		}

		return id + 1;
	}

	return token_id;
}

/*
 GetNextExpression�֐�
 �w�肳�ꂽ�g�[�N���̓��v�Z�D��x�̉E�̃g�[�N�����擾����
 ����
 parser		: �\����͊�
 token_id	: ���v�Z�D��x�̉E�̃g�[�N�����擾����g�[�N����ID
 �Ԃ�l
	���v�Z�D��x�̉E�̃g�[�N����ID
*/
static int GetNextExpression(SCRIPT_PARSER_ELEMENT* parser, int token_id)
{
	if(token_id < parser->num_tokens - 1)
	{
		if(parser->tokens[token_id]->token_type == TOKEN_TYPE_LEFT_PAREN)
		{
			int hierarchy = 1;
			int i;

			for(i = token_id + 1; i < parser->num_tokens; i++)
			{
				if(parser->tokens[i]->token_type == TOKEN_TYPE_LEFT_PAREN)
				{
					hierarchy++;
				}
				else if(parser->tokens[i]->token_type == TOKEN_TYPE_RIGHT_PAREN)
				{
					hierarchy--;
					if(hierarchy == 0)
					{
						return i + 1;
					}
				}
				else if(parser->tokens[i]->token_type == TOKEN_TYPE_LEFT_BRACE
					|| parser->tokens[i]->token_type == TOKEN_TYPE_RIGHT_BRACE)
				{
					parser->rule->error_message(parser->rule->error_message_data,
						parser->rule->file_names[parser->tokens[i]->file_id], parser->tokens[i]->line,
							"It is found that brace is invalid placed.\n");
					return -1;
				}
			}
		}
		else
		{
			if(parser->tokens[token_id + 1]->token_type != TOKEN_TYPE_FUNCTION)
			{
				return token_id + 1;
			}
		}
	}

	return 0;
}

/*
 ScriptBasicParserParseOneStepRecursive�֐�
 �f�t�H���g�̍\����͊��1�X�e�b�v���̍\����͂����s
 ����
 parser		: �\����͊�
 parent		: ���ۍ\���؂̐e�m�[�h
 token_id	: ��͂���g�[�N����ID
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseOneStepRecursive(SCRIPT_BASIC_PARSER* parser, ABSTRACT_SYNTAX_TREE* parent, int token_id)
{
	int i;

	// ��ɑ�����������Ă���
	for(i =token_id + 1; i < parser->element.num_tokens; i++)
	{
		uint16 token_type = parser->element.tokens[i]->token_type;
		if(token_type == TOKEN_TYPE_SEMI_COLON)
		{
			break;
		}
		else if(token_type == TOKEN_TYPE_ASSIGN)
		{
			if(parser->element.parse_assign(parser, NULL, i) == FALSE)
			{
				return FALSE;
			}
		}
	}

	return parser->element.parse_assign(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParse�֐�
 �f�t�H���g�̍\����͊�ō\����͎��s
 ����
 parser	: �\����͊�
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParse(SCRIPT_BASIC_PARSER* parser)
{
	int token_id;

	for(token_id=0; token_id<parser->element.num_tokens; token_id++)
	{
		switch(parser->element.tokens[token_id]->token_type)
		{
		case TOKEN_TYPE_ASSIGN:
			if(ScriptBasicParserParseOneStepRecursive(parser, NULL, token_id) == FALSE)
			{
				return FALSE;
			}
			// �Z�~�R�����܂œǂݔ�΂�
			do
			{
				token_id++;
			} while(token_id < parser->element.num_tokens && parser->element.tokens[token_id]->token_type != TOKEN_TYPE_SEMI_COLON);
			break;
		}
	}

	return TRUE;
}

/*
 ScriptBasicParserParseAssign�֐�
 �f�t�H���g�̍\����͊�ő�����������
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseAssign(SCRIPT_BASIC_PARSER* parser,int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_ASSIGN
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->assign_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}
		
		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		FLAG_ON(parser->token_check_flag, token_id);
		next_id = token_id + 2;

		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, next_id) == FALSE)
		{
			return FALSE;
		}
	}

	return parser->element.parse_bool_or(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseBoolOr�֐�
 �f�t�H���g�̍\����͊�Ř_���a�����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseBoolOr(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_BOOL_OR
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->bool_or_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			if(parser->element.tokens[next_id]->token_type == TOKEN_TYPE_BOOL_OR)
			{
				parser->new_parent = NULL;
				if(parser->element.parse_bool_or(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
			}
		}
		else if(next_id < 0)
		{
			return FALSE;
		}
		
		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}

		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		parser->new_parent = parent->left;
		FLAG_ON(parser->token_check_flag, token_id);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, GetLeftTokenID(parser->element.tokens, token_id - 1)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, token_id + 2) == FALSE)
		{
			return FALSE;
		}
	}

	return parser->element.parse_bool_and(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseBoolAnd�֐�
 �f�t�H���g�̍\����͊�Ř_���ς����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseBoolAnd(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_BOOL_AND
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->bool_and_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			if(parser->element.tokens[next_id]->token_type == TOKEN_TYPE_BOOL_AND)
			{
				parser->new_parent = NULL;
				if(parser->element.parse_bool_and(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
			}
		}
		else if(next_id < 0)
		{
			return FALSE;
		}
		
		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}

		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		parser->new_parent = parent->left;
		FLAG_ON(parser->token_check_flag, token_id);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, GetLeftTokenID(parser->element.tokens, token_id - 1)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, token_id + 2) == FALSE)
		{
			return FALSE;
		}
	}

	if(parser->element.parse_equal(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	return parser->element.parse_not_equal(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseEqual�֐�
 �f�t�H���g�̍\����͊�œ��l��r�����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseEqual(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_EQUAL
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->equal_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			switch(parser->element.tokens[next_id]->token_type)
			{
			case TOKEN_TYPE_EQUAL:
				parser->new_parent = NULL;
				if(parser->element.parse_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_NOT_EQUAL:
				parser->new_parent = NULL;
				if(parser->element.parse_not_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			}
		}
		else if(next_id < 0)
		{
			return FALSE;
		}
		
		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}

		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		parser->new_parent = parent->left;
		FLAG_ON(parser->token_check_flag, token_id);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, GetLeftTokenID(parser->element.tokens, token_id - 1)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, token_id + 2) == FALSE)
		{
			return FALSE;
		}
	}

	if(parser->element.parse_less(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	if(parser->element.parse_less_equal(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	if(parser->element.parse_greater(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	return parser->element.parse_greater_equal(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseNotEqual�֐�
 �f�t�H���g�̍\����͊�ŕs���l��r�����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseNotEqual(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_NOT_EQUAL
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->not_equal_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			switch(parser->element.tokens[next_id]->token_type)
			{
			case TOKEN_TYPE_EQUAL:
				parser->new_parent = NULL;
				if(parser->element.parse_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_NOT_EQUAL:
				parser->new_parent = NULL;
				if(parser->element.parse_not_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			}
		}
		else if(next_id < 0)
		{
			return FALSE;
		}
		
		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}

		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		parser->new_parent = parent->left;
		FLAG_ON(parser->token_check_flag, token_id);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, GetLeftTokenID(parser->element.tokens, token_id - 1)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, token_id + 2) == FALSE)
		{
			return FALSE;
		}
	}

	if(parser->element.parse_less(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	if(parser->element.parse_less_equal(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	if(parser->element.parse_greater(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	return parser->element.parse_greater_equal(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseLess�֐�
 �f�t�H���g�̍\����͊�ő召��r(����)�����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseLess(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_LESS
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->less_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			switch(parser->element.tokens[next_id]->token_type)
			{
			case TOKEN_TYPE_LESS:
				parser->new_parent = NULL;
				if(parser->element.parse_less(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_LESS_EQUAL:
				parser->new_parent = NULL;
				if(parser->element.parse_less_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER:
				parser->new_parent = NULL;
				if(parser->element.parse_greater(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER_EQUAL:
				parser->new_parent = NULL;
				if(parser->element.parse_greater_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			}
		}
		else if(next_id < 0)
		{
			return FALSE;
		}
		
		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}

		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		parser->new_parent = parent->left;
		FLAG_ON(parser->token_check_flag, token_id);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, GetLeftTokenID(parser->element.tokens, token_id - 1)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, token_id + 2) == FALSE)
		{
			return FALSE;
		}
	}

	if(parser->element.parse_multi(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	return parser->element.parse_divide(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseLessEqual�֐�
 �f�t�H���g�̍\����͊�ő召��r(�ȉ�)�����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseLessEqual(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_LESS_EQUAL
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->less_equal_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			switch(parser->element.tokens[next_id]->token_type)
			{
			case TOKEN_TYPE_LESS:
				parser->new_parent = NULL;
				if(parser->element.parse_less(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_LESS_EQUAL:
				parser->new_parent = NULL;
				if(parser->element.parse_less_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER:
				parser->new_parent = NULL;
				if(parser->element.parse_greater(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER_EQUAL:
				parser->new_parent = NULL;
				if(parser->element.parse_greater_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			}
		}
		else if(next_id < 0)
		{
			return FALSE;
		}
		
		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}

		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		parser->new_parent = parent->left;
		FLAG_ON(parser->token_check_flag, token_id);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, GetLeftTokenID(parser->element.tokens, token_id - 1)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, token_id + 2) == FALSE)
		{
			return FALSE;
		}
	}

	if(parser->element.parse_multi(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	return parser->element.parse_divide(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseGreater�֐�
 �f�t�H���g�̍\����͊�ő召��r(���z)�����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseGreater(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_GREATER
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->greater_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			switch(parser->element.tokens[next_id]->token_type)
			{
			case TOKEN_TYPE_LESS:
				parser->new_parent = NULL;
				if(parser->element.parse_less(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_LESS_EQUAL:
				parser->new_parent = NULL;
				if(parser->element.parse_less_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER:
				parser->new_parent = NULL;
				if(parser->element.parse_greater(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER_EQUAL:
				parser->new_parent = NULL;
				if(parser->element.parse_greater_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			}
		}
		else if(next_id < 0)
		{
			return FALSE;
		}
		
		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}

		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		parser->new_parent = parent->left;
		FLAG_ON(parser->token_check_flag, token_id);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, GetLeftTokenID(parser->element.tokens, token_id - 1)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, token_id + 2) == FALSE)
		{
			return FALSE;
		}
	}

	if(parser->element.parse_multi(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	return parser->element.parse_divide(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseGreaterEqual�֐�
 �f�t�H���g�̍\����͊�ő召��r(�ȏ�)�����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseGreaterEqual(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_GREATER_EQUAL
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->greater_equal_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			switch(parser->element.tokens[next_id]->token_type)
			{
			case TOKEN_TYPE_LESS:
				parser->new_parent = NULL;
				if(parser->element.parse_less(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_LESS_EQUAL:
				parser->new_parent = NULL;
				if(parser->element.parse_less_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER:
				parser->new_parent = NULL;
				if(parser->element.parse_greater(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER_EQUAL:
				parser->new_parent = NULL;
				if(parser->element.parse_greater_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			}
		}
		else if(next_id < 0)
		{
			return FALSE;
		}
		
		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}

		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		parser->new_parent = parent->left;
		FLAG_ON(parser->token_check_flag, token_id);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, GetLeftTokenID(parser->element.tokens, token_id - 1)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, token_id + 2) == FALSE)
		{
			return FALSE;
		}
	}

	if(parser->element.parse_multi(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	return parser->element.parse_divide(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseMulti�֐�
 �f�t�H���g�̍\����͊�Ŋ|���Z�����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseMulti(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_MULTI
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->multi_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			switch(parser->element.tokens[next_id]->token_type)
			{
			case TOKEN_TYPE_MULTI:
				parser->new_parent = NULL;
				if(parser->element.parse_multi(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_DIVIDE:
				parser->new_parent = NULL;
				if(parser->element.parse_divide(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			}
		}
		else if(next_id < 0)
		{
			return FALSE;
		}
		
		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}

		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		parser->new_parent = parent->left;
		FLAG_ON(parser->token_check_flag, token_id);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, GetLeftTokenID(parser->element.tokens, token_id - 1)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, token_id + 2) == FALSE)
		{
			return FALSE;
		}
	}

	if(parser->element.parse_plus(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	return parser->element.parse_minus(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseDivide�֐�
 �f�t�H���g�̍\����͊�Ŋ���Z�����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseDivide(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_DIVIDE
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;
		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->divide_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			switch(parser->element.tokens[next_id]->token_type)
			{
			case TOKEN_TYPE_MULTI:
				parser->new_parent = NULL;
				if(parser->element.parse_multi(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_DIVIDE:
				parser->new_parent = NULL;
				if(parser->element.parse_divide(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			}
		}
		else if(next_id < 0)
		{
			return FALSE;
		}
		
		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}

		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		parser->new_parent = parent->left;
		FLAG_ON(parser->token_check_flag, token_id);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, GetLeftTokenID(parser->element.tokens, token_id - 1)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, token_id + 2) == FALSE)
		{
			return FALSE;
		}
	}

	if(parser->element.parse_plus(&parser->element, token_id, parent) == FALSE)
	{
		return FALSE;
	}
	return parser->element.parse_minus(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParsePlus�֐�
 �f�t�H���g�̍\����͊�ő����Z�����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParsePlus(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_PLUS
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->plus_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			switch(parser->element.tokens[next_id]->token_type)
			{
			case TOKEN_TYPE_PLUS:
				parser->new_parent = NULL;
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				parser->new_parent = NULL;
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			}
		}
		else if(next_id < 0)
		{
			return FALSE;
		}
		
		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}

		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		parser->new_parent = parent->left;
		FLAG_ON(parser->token_check_flag, token_id);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, GetLeftTokenID(parser->element.tokens, token_id - 1)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, token_id + 2) == FALSE)
		{
			return FALSE;
		}
	}

	return parser->element.parse_paren(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseMinus�֐�
 �f�t�H���g�̍\����͊�ň����Z�����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseMinus(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_MINUS
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->minus_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			switch(parser->element.tokens[next_id]->token_type)
			{
			case TOKEN_TYPE_PLUS:
				parser->new_parent = NULL;
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				parser->new_parent = NULL;
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if(parser->new_parent != NULL)
				{
					parent = parser->new_parent;
				}
				break;
			}
		}
		else if(next_id < 0)
		{
			return FALSE;
		}
		
		if(parent == NULL)
		{
			parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
			PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		}

		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		parser->new_parent = parent->left;
		FLAG_ON(parser->token_check_flag, token_id);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, GetLeftTokenID(parser->element.tokens, token_id - 1)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, token_id + 2) == FALSE)
		{
			return FALSE;
		}
	}

	return parser->element.parse_paren(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseParen�֐�
 �f�t�H���g�̍\����͊��( )�����
 ����
 parser		: �\����͊�
 token_id	: ��͒��̃g�[�N��ID
 parent		: ���ۍ\���؂̐e�m�[�h
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
static int ScriptBasicParserParseParen(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(token_id > 0 && FLAG_CHECK(parser->token_check_flag, token_id - 1) == 0)
	{
		if(parser->element.tokens[token_id - 1]->token_type == TOKEN_TYPE_LEFT_PAREN)
		{
			TOKEN *left, *right;
			int hierarchy;
			int i;

			left = (token_id == 0) ? NULL : parser->element.tokens[token_id];
			right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 2];

			FLAG_ON(parser->token_check_flag, token_id - 1);

			hierarchy = 1;
			for(i=token_id; i<parser->element.num_tokens; i++)
			{
				int token_type = parser->element.tokens[i]->token_type;
				if(token_type == TOKEN_TYPE_LEFT_PAREN)
				{
					hierarchy++;
				}
				else if(token_type == TOKEN_TYPE_RIGHT_PAREN)
				{
					hierarchy--;
					if(hierarchy == 0)
					{
						parser->new_parent = NULL;
						if(ScriptBasicParserParseOneStepRecursive(parser, parent, i + 1) == FALSE)
						{
							return FALSE;
						}
						if(parser->new_parent != NULL)
						{
							parent = parser->new_parent;
						}
						break;
					}
				}
				else if(token_type == TOKEN_TYPE_SEMI_COLON)
				{
					break;
				}
			}
			if(left != NULL)
			{
				FLAG_ON(parser->token_check_flag, token_id - 1);
				if(ScriptBasicParserParseOneStepRecursive(parser, parent, token_id + 1) == FALSE)
				{
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

/*
 ReleaseScriptBasicParser�֐�
 �f�t�H���g�̍\����͊�̃��������J��
 ����
 parser	: �\����͊�
*/
static void ReleaseScriptBasicParser(SCRIPT_BASIC_PARSER* parser)
{
	MEM_FREE_FUNC(parser->token_check_flag);
	ReleasePointerArray(&parser->element.abstract_syntax_tree);
}

/*
 InitializeScriptBasicParser�֐�
 �f�t�H���g�̍\����͊��������
 ����
 parser				: �\����͊�
 rule				: �\����͂Ɏg�p����\���`�F�b�N�p�f�[�^
 tokens				: �����͂ɂ���ē���ꂽ�g�[�N���z��
 num_tokens			: �����͂ɂ���ē���ꂽ�g�[�N���̐�
 user_functions		: ���[�U�[��`�֐��̖��O�z��
 num_user_functions	: ���[�U�[��`�֐��̐�
 user_data			: ���[�U�[��`�֐��Ŏg�p���郆�[�U�[�f�[�^
*/
void InitializeScriptBasicParser(
	SCRIPT_BASIC_PARSER* parser,
	SCRIPT_RULE_ELEMENT* rule,
	TOKEN** tokens,
	int num_tokens,
	const char** user_functions,
	int num_user_functions,
	void* user_data
)
{
#define TREE_BLOCK_SIZE 1024
	(void)memset(parser, 0, sizeof(*parser));
	parser->token_check_flag = (unsigned int*)MEM_ALLOC_FUNC((num_tokens + sizeof(*parser->token_check_flag) * 8 - 1) / (sizeof(*parser->token_check_flag) * 8) * sizeof(*parser->token_check_flag));
	(void)memset(parser->token_check_flag, 0, (num_tokens + sizeof(*parser->token_check_flag) * 8 - 1) / (sizeof(*parser->token_check_flag) * 8) * sizeof(*parser->token_check_flag));
	InitializePointerArray(&parser->element.abstract_syntax_tree, TREE_BLOCK_SIZE, NULL);
	parser->element.rule = rule;
	parser->element.memory_pool = rule->memory_pool;
	parser->element.user_functions = user_functions;
	parser->element.num_user_functions = num_user_functions;
	parser->element.user_data = user_data;

	parser->element.tokens = tokens;
	parser->element.num_tokens = num_tokens;
	parser->element.parse = (int (*)(SCRIPT_PARSER_ELEMENT*))ScriptBasicParserParse;
	parser->element.parse_assign = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseAssign;
	parser->element.parse_bool_or = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseBoolOr;
	parser->element.parse_bool_and = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseBoolAnd;
	parser->element.parse_equal = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseEqual;
	parser->element.parse_not_equal = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseNotEqual;
	parser->element.parse_less = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseLess;
	parser->element.parse_less_equal = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseLessEqual;
	parser->element.parse_greater = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseGreater;
	parser->element.parse_greater_equal = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseGreaterEqual;
	parser->element.parse_multi = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseMulti;
	parser->element.parse_divide = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseDivide;
	parser->element.parse_plus = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParsePlus;
	parser->element.parse_minus = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseMinus;
	parser->element.parse_paren = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseParen;
	parser->element.release = (void (*)(SCRIPT_PARSER_ELEMENT*))ReleaseScriptBasicParser;
#undef TREE_BLOCK_SIZE
}

#ifdef __cplusplus
}
#endif
