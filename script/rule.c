#include <string.h>
#include "rule.h"
#include "lexical_analyser.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 ScriptRuleElementSetReservedRule�֐�
 �\����͂ɗ\���̃��[����ݒ肷��
 ����
 element		: �X�N���v�g�̍\����̓��[���̊�{�f�[�^
 rules			: �\���̃��[�������֐�
 function_data	: ���[���������s���Ɏg�p����f�[�^
*/
void ScriptRuleElementSetReservedRule(
	SCRIPT_RULE_ELEMENT* element,
	int (**rules)(struct _SCRIPT_RULE_ELEMENT* rule, struct _LEXICAL_ANALYSER* analyser, void* function_data),
	void* function_data
)
{
	element->reserved_rule = rules;
	element->function_data = function_data;
}

/*
 ScriptBasicAssignRule�֐�
 ����̃��[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicAssignRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(left == NULL || right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Assign operand must have left side token and right side token.");
		}
		return FALSE;
	}

	if(left->token_type != TOKEN_TYPE_IDENT)
	{
		if(rule->element.error_message != NULL)
		{
			rule->element.error_message(rule->element.error_message_data,
				rule->element.file_names[left->file_id], left->line, "Left side token of assign operand must be Variable.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicPlusRule�֐�
 �����Z�܂��̓v���X�����̃��[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicPlusRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Plus operand must have right side token.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicMinusRule�֐�
 �����Z�܂��̓}�C�i�X�����̃��[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicMinusRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Minus operand must have right side token.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicMultiRule�֐�
 �|���Z�̃��[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicMultiRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(left == NULL || right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Multi operand must have left side token and right side token.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicDivideRule�֐�
 ����Z�̃��[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicDivideRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(left == NULL || right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Divide operand must have left side token and right side token.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicModRule�֐�
 �]��v�Z�̃��[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicModRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(left == NULL || right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Mod operand must have left side token and right side token.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicBoolOrRule�֐�
 �_���a���Z���[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicBoolOrRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(left == NULL || right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Boolean OR operand must have left side token and right side token.");
		}
		return FALSE;
	}

	if(left->token_type == TOKEN_TYPE_CONST_STRING
		|| right->token_type == TOKEN_TYPE_CONST_STRING
		|| left->token_type == TOKEN_TYPE_CONST_FLOAT
		|| right->token_type == TOKEN_TYPE_CONST_FLOAT
	)
	{
		if(rule->element.error_message != NULL)
		{
			rule->element.error_message(rule->element.error_message_data,
				rule->element.file_names[left->file_id], left->line, "Invalid token for OR operand.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicBoolAndRule�֐�
 �_���a���Z���[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicBoolAndRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(left == NULL || right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Boolean AND operand must have left side token and right side token.");
		}
		return FALSE;
	}

	if(left->token_type == TOKEN_TYPE_CONST_STRING
		|| right->token_type == TOKEN_TYPE_CONST_STRING
		|| left->token_type == TOKEN_TYPE_CONST_FLOAT
		|| right->token_type == TOKEN_TYPE_CONST_FLOAT
	)
	{
		if(rule->element.error_message != NULL)
		{
			rule->element.error_message(rule->element.error_message_data,
				rule->element.file_names[left->file_id], left->line, "Invalid token for OR operand.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicEqualRule�֐�
 ���l��r���[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicEqualRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(left == NULL || right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Equal operand must have left side token and right side token.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicNotEqualRule�֐�
 �s���l��r���[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicNotEqualRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(left == NULL || right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Not equal operand must have left side token and right side token.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicLessRule�֐�
 �l��r(����)���[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicLessRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(left == NULL || right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Less operand must have left side token and right side token.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicLesslEqualRule�֐�
 �l��r(�ȉ�)���[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicLessEqualRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(left == NULL || right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Less equal operand must have left side token and right side token.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicGreaterRule�֐�
 �l��r(���z)���[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicGreaterRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(left == NULL || right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Greater operand must have left side token and right side token.");
		}
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicGreaterEqualRule�֐�
 �l��r(�ȏ�)���[��
 ����
 rule	: �\����͂̃��[�����Ǘ�����f�[�^
 left	: ���ӂ̃g�[�N��
 right	: �E�ӂ̃g�[�N��
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
static int ScriptBasicGreaterEqualRule(SCRIPT_BASIC_RULE* rule, TOKEN* left, TOKEN* right)
{
	if(left == NULL || right == NULL)
	{
		if(rule->element.error_message != NULL)
		{
			const char *file_name = "";
			int line = 0;
			if(left != NULL)
			{
				file_name = rule->element.file_names[left->file_id];
				line = left->length;
			}
			else if(right != NULL)
			{
				file_name = rule->element.file_names[right->file_id];
				line = right->line;
			}

			rule->element.error_message(rule->element.error_message_data,
				file_name, line, "Greater equal operand must have left side token and right side token.");
		}
		return FALSE;
	}

	return TRUE;
}

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
void InitializeScriptBasicRule(
	SCRIPT_BASIC_RULE* rule,
	void (*error_message)(void* error_message_data, const char* file_name, int line, const char* message, ...),
	void* error_message_data,
	const char** file_names,
	MEMORY_POOL* memory_pool
)
{
	(void)memset(rule, 0, sizeof(*rule));

	rule->element.error_message = error_message;
	rule->element.error_message_data = error_message_data;
	rule->element.file_names = file_names;
	rule->element.memory_pool = memory_pool;

	rule->element.assign_rule =
		(int (*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicAssignRule;
	rule->element.plus_rule =
		(int(*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicPlusRule;
	rule->element.minus_rule =
		(int(*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicMinusRule;
	rule->element.multi_rule =
		(int(*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicMultiRule;
	rule->element.divide_rule =
		(int(*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicDivideRule;
	rule->element.mod_rule =
		(int(*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicModRule;
	rule->element.bool_or_rule =
		(int (*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicBoolOrRule;
	rule->element.bool_and_rule =
		(int (*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicBoolAndRule;
	rule->element.equal_rule =
		(int (*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicEqualRule;
	rule->element.not_equal_rule =
		(int (*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicNotEqualRule;
	rule->element.less_rule =
		(int (*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicLessRule;
	rule->element.less_equal_rule =
		(int (*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicLessEqualRule;
	rule->element.greater_rule =
		(int (*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicGreaterRule;
	rule->element.greater_equal_rule =
		(int (*)(SCRIPT_RULE_ELEMENT*, TOKEN*, TOKEN*))ScriptBasicGreaterEqualRule;
}

/*
 ScriptBasicIfRule�֐�
 if����\�����[��
 ����
 rule		: �\����͂̃��[�����Ǘ�����f�[�^
 analyser		: �����͊�
 function_data	: �_�~�[
 �Ԃ�l
	���[���𖞑�:TRUE	���[���𖞂����Ȃ�:FALSE
*/
int ScriptBasicIfRule(struct _SCRIPT_RULE_ELEMENT* rule, struct _LEXICAL_ANALYSER* analyser, void* function_data)
{
	TOKEN *next_token = LexicalAnalyserReadToken(analyser);
	if(next_token->token_type != TOKEN_TYPE_LEFT_PAREN)
	{
		const char *file_name = "";
		int line = 0;

		file_name = rule->file_names[next_token->file_id];
		line = next_token->length;
		rule->error_message(rule->error_message_data,
			file_name, line, "\"(\" is missed after \"if\"...");
		return FALSE;
	}
	return TRUE;
}

#ifdef __cplusplus
}
#endif
