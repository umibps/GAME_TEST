#include <string.h>
#include "rule.h"
#include "lexical_analyser.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 ScriptRuleElementSetReservedRule関数
 構文解析に予約語のルールを設定する
 引数
 element		: スクリプトの構文解析ルールの基本データ
 rules			: 予約語のルール処理関数
 function_data	: ルール処理実行時に使用するデータ
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
 ScriptBasicAssignRule関数
 代入のルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicPlusRule関数
 足し算またはプラス符号のルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicMinusRule関数
 引き算またはマイナス符号のルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicMultiRule関数
 掛け算のルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicDivideRule関数
 割り算のルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicModRule関数
 余り計算のルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicBoolOrRule関数
 論理和演算ルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicBoolAndRule関数
 論理和演算ルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicEqualRule関数
 同値比較ルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicNotEqualRule関数
 不同値比較ルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicLessRule関数
 値比較(未満)ルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicLesslEqualRule関数
 値比較(以下)ルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicGreaterRule関数
 値比較(超越)ルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 ScriptBasicGreaterEqualRule関数
 値比較(以上)ルール
 引数
 rule	: 構文解析のルールを管理するデータ
 left	: 左辺のトークン
 right	: 右辺のトークン
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
 InitializeScriptBasicRule関数
 デフォルトの構文チェック処理用データを初期化
 引数
 rule				: デフォルトの構文チェック処理用データ
 error_message		: エラーメッセージ表示用関数
 error_message_data	: エラーメッセージ表示用関数で使うユーザーデータ
 file_names			: スクリプトファイル名の配列
 memory_pool		: メモリ管理用データ
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
 ScriptBasicIfRule関数
 if制御構文ルール
 引数
 rule		: 構文解析のルールを管理するデータ
 analyser		: 字句解析器
 function_data	: ダミー
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
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
