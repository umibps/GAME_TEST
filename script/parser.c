#include <string.h>
#include <stdint.h>
#include "parser.h"

#define SCRIPT_PARSER_MEMORY_POOL_SIZE 4096

#ifdef __cplusplus
extern "C" {
#endif

/*
 eCALCULATE_PRIORITY列挙体
 計算優先度
*/
typedef enum _eCALCULATE_PRIORITY
{
	CALCULATE_PRIORITY_OTHER,
	CALCULATE_PRIORITY_BOOL_OR,
	CALCULATE_PRIORITY_BOOL_AND,
	CALCULATE_PRIORITY_COMPARE,
	CALCULATE_PRIORITY_MULTI_DIVIDE,
	CALCULATE_PRIORITY_PLUS_MINUS
} eCALCULATE_PRIORITY;

/*
 ScriptParserElementNewLeafBinary関数
 2項演算に対して抽象構文木を新たに確保する
 引数
 parser	: 構文解析器
 parent	: 親ノード
 left	: 左側の葉
 right	: 右側の葉
*/
void ScriptParserElementNewLeafBinary(SCRIPT_PARSER_ELEMENT* parser, ABSTRACT_SYNTAX_TREE* parent, TOKEN* left, TOKEN* right)
{
	parent->left = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->memory_pool, sizeof(*parent->left));
	parent->left->token = left;
	parent->left->left = parent->left->center = parent->left->right = NULL;
	parent->right = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->memory_pool, sizeof(*parent->right));
	parent->right->token = right;
	parent->right->left = parent->right->center = parent->right->right = NULL;
	parent->center = NULL;
}

/*
 ScriptParserElementSetReservedParseRule関数
 構文解析器に予約語及び組み込み関数の構文解析用のルールを追加する
 引数
 parser				: 構文解析器
 parse_functions	: 構文解析処理を行う関数ポインタ配列
*/
void ScriptParserElementSetReservedParseRule(
	SCRIPT_PARSER_ELEMENT* parser,
	int (**parse_functions)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
)
{
	parser->parse_reserved = parse_functions;
}

/*
 GetCalculatePriority関数
 トークンのタイプから計算優先度を取得する
 token_type	: トークンのタイプ
 返り値
	計算優先度
*/
static eCALCULATE_PRIORITY GetCalculatePriority(int token_type)
{
	switch(token_type)
	{
	case TOKEN_TYPE_BOOL_OR:
		return CALCULATE_PRIORITY_BOOL_OR;
	case TOKEN_TYPE_BOOL_AND:
		return CALCULATE_PRIORITY_BOOL_AND;
	case TOKEN_TYPE_EQUAL:
	case TOKEN_TYPE_NOT_EQUAL:
	case TOKEN_TYPE_LESS:
	case TOKEN_TYPE_LESS_EQUAL:
	case TOKEN_TYPE_GREATER:
	case TOKEN_TYPE_GREATER_EQUAL:
		return CALCULATE_PRIORITY_BOOL_AND;
	}
	return CALCULATE_PRIORITY_OTHER;
}

/*
 IsAbstractTreeLeafToken関数
 トークンが抽象構文木の終端であるかを取得する
 引数
 token_type	: トークンのタイプ
 返り値
	終端:TRUE	終端ではない:FALSE
*/
static int IsAbstractTreeLeafToken(int token_type)
{
	if(token_type == TOKEN_TYPE_CONST_DIGIT
		|| token_type == TOKEN_TYPE_CONST_FLOAT
		|| token_type == TOKEN_TYPE_CONST_STRING
		|| token_type == TOKEN_TYPE_IDENT
	)
	{
		return TRUE;
	}

	return FALSE;
}

/*
 GetLeftTokenID関数
 指定されたトークンの左側のトークンを取得する
 小括弧は読み飛ばす
 引数
 tokens		: 字句解析によって得られたトークン
 token_id	: 指定するトークンのID
 返り値
	指定されたトークンの左側のトークンのID
*/
static int GetLeftTokenID(TOKEN** tokens, int token_id, eCALCULATE_PRIORITY priority)
{
	int id = token_id;
	do
	{
		if(tokens[token_id]->token_type == TOKEN_TYPE_RIGHT_PAREN)
		{
			int hierarchy = 1;
			id = token_id - 1;
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

			id++;
		}
	} while(GetCalculatePriority(tokens[id]->token_type) == priority);

	return id;
}

/*
GetRightTokenID関数
 指定されたトークンの右側のトークンを取得する
 小括弧は読み飛ばす
 引数
 tokens		: 字句解析によって得られたトークン
 token_id	: 指定するトークンのID
 num_tokens	: 取得したトークンの数
 返り値
	指定されたトークンの右側のトークンのID
*/
static int GetRightTokenID(TOKEN** tokens,int token_id, int num_tokens)
{
	if(token_id <= num_tokens - 3)
	{
		if(tokens[token_id + 1]->token_type == TOKEN_TYPE_LEFT_PAREN)
		{
			return token_id + 3;
		}
		return token_id + 2;
	}
	else if(token_id <= num_tokens - 2)
	{
		return token_id + 2;
	}
	else if(token_id <= num_tokens - 1)
	{
		return token_id;
	}

	return token_id;
}

/*
 GetNextExpression関数
 指定されたトークンの同計算優先度の右のトークンを取得する
 引数
 parser		: 構文解析器
 token_id	: 同計算優先度の右のトークンを取得するトークンのID
 返り値
	同計算優先度の右のトークンのID
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
 ScriptBasicParserParseOneStepRecursive関数
 デフォルトの構文解析器で1ステップ分の構文解析を実行
 引数
 parser		: 構文解析器
 parent		: 抽象構文木の親ノード
 token_id	: 解析するトークンのID
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseOneStepRecursive(SCRIPT_BASIC_PARSER* parser, ABSTRACT_SYNTAX_TREE* parent, int token_id)
{
	int hierarchy = 0;
	int i;

	// セミコロンなら命令終了
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_SEMI_COLON
		|| parser->element.tokens[token_id]->token_type == TOKEN_TYPE_RIGHT_PAREN)
	{
		return TRUE;
	}

	// 先に代入を処理しておく
	for(i =token_id + 1; i < parser->element.num_tokens; i++)
	{
		uint16 token_type = parser->element.tokens[i]->token_type;
		if(token_type == TOKEN_TYPE_SEMI_COLON || token_type == TOKEN_TYPE_COMMA)
		{
			FLAG_ON(parser->token_check_flag, i);
			break;
		}
		else if(token_type == TOKEN_TYPE_LEFT_PAREN)
		{
			hierarchy++;
		}
		else if(token_type == TOKEN_TYPE_RIGHT_PAREN)
		{
			if(hierarchy <= 0)
			{
				break;
			}
			hierarchy--;
		}
		else if(token_type == TOKEN_TYPE_ASSIGN)
		{
			if(parser->element.parse_assign(&parser->element, i, NULL) == FALSE)
			{
				return FALSE;
			}
		}
		else if(token_type == TOKEN_TYPE_IDENT && (parser->flags & SCRIPT_BASIC_PARSER_FLAG_IN_ASSIGN) == FALSE)
		{
			if(parser->element.parse_function(&parser->element, i, parent) == FALSE)
			{
				return FALSE;
			}
		}
		else if(token_type >= NUM_DEFAULT_TOKEN_TYPE)
		{
			int reserved_id = token_type - NUM_DEFAULT_TOKEN_TYPE;
			if(reserved_id >= 0 && reserved_id < parser->element.rule->num_reserved_rule)
			{
				if(parser->element.parse_reserved[reserved_id](&parser->element, i, NULL) == FALSE)
				{
					return FALSE;
				}
			}
		}
	}

	return parser->element.parse_assign(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParse関数
 デフォルトの構文解析器で構文解析実行
 引数
 parser	: 構文解析器
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParse(SCRIPT_BASIC_PARSER* parser)
{
	int token_id;

	for(token_id=0; token_id<parser->element.num_tokens; token_id++)
	{
		switch(parser->element.tokens[token_id]->token_type)
		{
		case TOKEN_TYPE_ASSIGN:
			parser->flags |= SCRIPT_BASIC_PARSER_FLAG_IN_ASSIGN;
			if(ScriptBasicParserParseOneStepRecursive(parser, NULL, token_id) == FALSE)
			{
				return FALSE;
			}
			parser->flags &= ~(SCRIPT_BASIC_PARSER_FLAG_IN_ASSIGN);
			// セミコロンまで読み飛ばす
			do
			{
				token_id++;
			} while(token_id < parser->element.num_tokens && parser->element.tokens[token_id]->token_type != TOKEN_TYPE_SEMI_COLON);
			break;
		case TOKEN_TYPE_LEFT_BRACE:
		case TOKEN_TYPE_RIGHT_BRACE:
			parser->element.parse_brace(&parser->element, token_id, NULL);
			break;
		default:
			if(FLAG_CHECK(parser->token_check_flag, token_id) == FALSE)
			{
				int reserved_id;
				if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_IDENT)
				{
					TOKEN *right = (token_id < parser->element.num_tokens - 1) ? parser->element.tokens[token_id + 1] : NULL;
					parser->flags |= SCRIPT_BASIC_PARSER_FLAG_WASTE_RETURN;
					if(parser->element.parse_function(&parser->element, token_id, NULL) == FALSE)
					{
						return FALSE;
					}
					parser->flags &= ~(SCRIPT_BASIC_PARSER_FLAG_WASTE_RETURN);
					if(IsAbstractTreeLeafToken(parser->element.tokens[token_id]->token_type) != FALSE)
					{
						if(right != NULL && (right->token_type == TOKEN_TYPE_SEMI_COLON || right->token_type == TOKEN_TYPE_COMMA))
						{
							ABSTRACT_SYNTAX_TREE *tree = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*tree));
							tree->left = tree->center = tree->right = NULL;
							tree->token = parser->element.tokens[token_id];
							PointerArrayAppend(&parser->element.abstract_syntax_tree, tree);
							FLAG_ON(parser->token_check_flag, token_id);
						}
					}
					if(parser->element.tokens[token_id]->token_type == SCRIPT_BASIC_RESERVED_USER_FUNCTION)
					{
						while(FLAG_CHECK(parser->token_check_flag, token_id) != FALSE)
						{
							token_id++;
						}
						token_id = token_id - 1;
					}
				}
				reserved_id = parser->element.tokens[token_id]->token_type - NUM_DEFAULT_TOKEN_TYPE;
				if(reserved_id >= 0 && reserved_id < parser->element.rule->num_reserved_rule)
				{
					if(parser->element.parse_reserved[reserved_id](
						&parser->element, token_id, NULL) == FALSE)
					{
						return FALSE;
					}
					while(FLAG_CHECK(parser->token_check_flag, token_id) != FALSE)
					{
						token_id++;
					}
					token_id = token_id - 1;
				}
				break;
			}
		}
	}

	return TRUE;
}

/*
 ScriptBasicParserParseAssign関数
 デフォルトの構文解析器で代入処理を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseAssign(SCRIPT_BASIC_PARSER* parser,int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_ASSIGN
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		uintptr_t function_id;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

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

		next_id = token_id + 2;

		if((function_id = (uintptr_t)StringHashTableGet(parser->element.user_functions,right->name)) != 0)
		{
			parser->element.parse_function(&parser->element, token_id + 1, parent->right);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, next_id) == FALSE)
		{
			return FALSE;
		}

		if(left != NULL && IsAbstractTreeLeafToken(left->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id + 1);
		}
		if(right != NULL && IsAbstractTreeLeafToken(right->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id - 1);
		}
	}

	return parser->element.parse_bool_or(&parser->element, token_id, parent);
}

/*
 ScriptBasicParserParseBoolOr関数
 デフォルトの構文解析器で論理和を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseBoolOr(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_BOOL_OR
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left = NULL, *right = NULL;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->bool_or_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			switch(parser->element.tokens[next_id]->token_type)
			{
				ABSTRACT_SYNTAX_TREE *new_parent;
			case TOKEN_TYPE_BOOL_OR:
				if(parser->element.parse_bool_or(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_BOOL_AND:
				if(parser->element.parse_bool_and(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_PLUS:
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MULTI:
				if(parser->element.parse_multi(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_DIVIDE:
				if(parser->element.parse_divide(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_EQUAL:
				if(parser->element.parse_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_NOT_EQUAL:
				if(parser->element.parse_not_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS:
				if(parser->element.parse_less(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS_EQUAL:
				if(parser->element.parse_less_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER:
				if(parser->element.parse_greater(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER_EQUAL:
				if(parser->element.parse_greater_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
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
		if(parent->left != NULL)
		{
			PointerArrayPush(&parser->new_parent, parent->left);
		}
		if(left != NULL && IsAbstractTreeLeafToken(left->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id + 1);
		}
		if(right != NULL && IsAbstractTreeLeafToken(right->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id - 1);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left,
			GetLeftTokenID(parser->element.tokens, token_id - 1, CALCULATE_PRIORITY_BOOL_OR)) == FALSE)
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
 ScriptBasicParserParseBoolAnd関数
 デフォルトの構文解析器で論理積を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseBoolAnd(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_BOOL_AND
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if(parser->element.rule->bool_and_rule(parser->element.rule, left, right) == FALSE)
		{
			return FALSE;
		}

		next_id = GetNextExpression(&parser->element, token_id + 1);
		if(next_id > 0)
		{
			switch(parser->element.tokens[next_id]->token_type)
			{
				ABSTRACT_SYNTAX_TREE *new_parent;
			case TOKEN_TYPE_BOOL_AND:
				if(parser->element.parse_bool_and(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_PLUS:
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MULTI:
				if(parser->element.parse_multi(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_DIVIDE:
				if(parser->element.parse_divide(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_EQUAL:
				if(parser->element.parse_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_NOT_EQUAL:
				if(parser->element.parse_not_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS:
				if(parser->element.parse_less(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS_EQUAL:
				if(parser->element.parse_less_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER:
				if(parser->element.parse_greater(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER_EQUAL:
				if(parser->element.parse_greater_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
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
		if(parent->left != NULL)
		{
			PointerArrayPush(&parser->new_parent, parent->left);
		}
		if(left != NULL && IsAbstractTreeLeafToken(left->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id + 1);
		}
		if(right != NULL && IsAbstractTreeLeafToken(right->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id - 1);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left,
			GetLeftTokenID(parser->element.tokens, token_id - 1, CALCULATE_PRIORITY_BOOL_AND)) == FALSE)
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
 ScriptBasicParserParseEqual関数
 デフォルトの構文解析器で同値比較を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseEqual(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_EQUAL
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

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
				ABSTRACT_SYNTAX_TREE *new_parent;
			case TOKEN_TYPE_PLUS:
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MULTI:
				if(parser->element.parse_multi(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_DIVIDE:
				if(parser->element.parse_divide(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_EQUAL:
				if(parser->element.parse_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_NOT_EQUAL:
				if(parser->element.parse_not_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS:
				if(parser->element.parse_less(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS_EQUAL:
				if(parser->element.parse_less_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER:
				if(parser->element.parse_greater(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER_EQUAL:
				if(parser->element.parse_greater_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
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
		if(parent->left != NULL)
		{
			PointerArrayPush(&parser->new_parent, parent->left);
		}
		if(left != NULL && IsAbstractTreeLeafToken(left->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id + 1);
		}
		if(right != NULL && IsAbstractTreeLeafToken(right->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id - 1);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left,
			GetLeftTokenID(parser->element.tokens, token_id - 1, CALCULATE_PRIORITY_COMPARE)) == FALSE)
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
 ScriptBasicParserParseNotEqual関数
 デフォルトの構文解析器で不同値比較を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseNotEqual(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_NOT_EQUAL
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

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
				ABSTRACT_SYNTAX_TREE *new_parent;
			case TOKEN_TYPE_PLUS:
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MULTI:
				if(parser->element.parse_multi(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_DIVIDE:
				if(parser->element.parse_divide(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_EQUAL:
				if(parser->element.parse_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_NOT_EQUAL:
				if(parser->element.parse_not_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS:
				if(parser->element.parse_less(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS_EQUAL:
				if(parser->element.parse_less_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER:
				if(parser->element.parse_greater(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER_EQUAL:
				if(parser->element.parse_greater_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
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
		if(parent->left != NULL)
		{
			PointerArrayPush(&parser->new_parent, parent->left);
		}
		if(left != NULL && IsAbstractTreeLeafToken(left->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id + 1);
		}
		if(right != NULL && IsAbstractTreeLeafToken(right->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id - 1);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left,
			GetLeftTokenID(parser->element.tokens, token_id - 1, CALCULATE_PRIORITY_COMPARE)) == FALSE)
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
 ScriptBasicParserParseLess関数
 デフォルトの構文解析器で大小比較(未満)を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseLess(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_LESS
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

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
				ABSTRACT_SYNTAX_TREE *new_parent;
			case TOKEN_TYPE_PLUS:
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MULTI:
				if(parser->element.parse_multi(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_DIVIDE:
				if(parser->element.parse_divide(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_EQUAL:
				if(parser->element.parse_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_NOT_EQUAL:
				if(parser->element.parse_not_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS:
				if(parser->element.parse_less(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS_EQUAL:
				if(parser->element.parse_less_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER:
				if(parser->element.parse_greater(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER_EQUAL:
				if(parser->element.parse_greater_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
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
		if(parent->left != NULL)
		{
			PointerArrayPush(&parser->new_parent, parent->left);
		}
		if(left != NULL && IsAbstractTreeLeafToken(left->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id + 1);
		}
		if(right != NULL && IsAbstractTreeLeafToken(right->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id - 1);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left,
			GetLeftTokenID(parser->element.tokens, token_id - 1, CALCULATE_PRIORITY_COMPARE)) == FALSE)
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
 ScriptBasicParserParseLessEqual関数
 デフォルトの構文解析器で大小比較(以下)を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseLessEqual(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_LESS_EQUAL
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

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
				ABSTRACT_SYNTAX_TREE *new_parent;
			case TOKEN_TYPE_PLUS:
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MULTI:
				if(parser->element.parse_multi(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_DIVIDE:
				if(parser->element.parse_divide(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_EQUAL:
				if(parser->element.parse_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_NOT_EQUAL:
				if(parser->element.parse_not_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS:
				if(parser->element.parse_less(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS_EQUAL:
				if(parser->element.parse_less_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER:
				if(parser->element.parse_greater(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER_EQUAL:
				if(parser->element.parse_greater_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
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
		if(parent->left != NULL)
		{
			PointerArrayPush(&parser->new_parent, parent->left);
		}
		if(left != NULL && IsAbstractTreeLeafToken(left->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id + 1);
		}
		if(right != NULL && IsAbstractTreeLeafToken(right->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id - 1);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left,
			GetLeftTokenID(parser->element.tokens, token_id - 1, CALCULATE_PRIORITY_COMPARE)) == FALSE)
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
 ScriptBasicParserParseGreater関数
 デフォルトの構文解析器で大小比較(超越)を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseGreater(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_GREATER
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

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
				ABSTRACT_SYNTAX_TREE *new_parent;
			case TOKEN_TYPE_PLUS:
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MULTI:
				if(parser->element.parse_multi(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_DIVIDE:
				if(parser->element.parse_divide(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_EQUAL:
				if(parser->element.parse_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_NOT_EQUAL:
				if(parser->element.parse_not_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS:
				if(parser->element.parse_less(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS_EQUAL:
				if(parser->element.parse_less_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER:
				if(parser->element.parse_greater(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER_EQUAL:
				if(parser->element.parse_greater_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
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
		if(parent->left != NULL)
		{
			PointerArrayPush(&parser->new_parent, parent->left);
		}
		if(left != NULL && IsAbstractTreeLeafToken(left->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id + 1);
		}
		if(right != NULL && IsAbstractTreeLeafToken(right->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id - 1);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left,
			GetLeftTokenID(parser->element.tokens, token_id - 1, CALCULATE_PRIORITY_COMPARE)) == FALSE)
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
 ScriptBasicParserParseGreaterEqual関数
 デフォルトの構文解析器で大小比較(以上)を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseGreaterEqual(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_GREATER_EQUAL
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

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
				ABSTRACT_SYNTAX_TREE *new_parent;
			case TOKEN_TYPE_PLUS:
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MULTI:
				if(parser->element.parse_multi(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_DIVIDE:
				if(parser->element.parse_divide(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_EQUAL:
				if(parser->element.parse_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_NOT_EQUAL:
				if(parser->element.parse_not_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS:
				if(parser->element.parse_less(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_LESS_EQUAL:
				if(parser->element.parse_less_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER:
				if(parser->element.parse_greater(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_GREATER_EQUAL:
				if(parser->element.parse_greater_equal(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
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
		if(parent->left != NULL)
		{
			PointerArrayPush(&parser->new_parent, parent->left);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left,
			GetLeftTokenID(parser->element.tokens, token_id - 1, CALCULATE_PRIORITY_COMPARE)) == FALSE)
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
 ScriptBasicParserParseMulti関数
 デフォルトの構文解析器で掛け算を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseMulti(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_MULTI
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

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
				ABSTRACT_SYNTAX_TREE *new_parent;
			case TOKEN_TYPE_PLUS:
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MULTI:
				if(parser->element.parse_multi(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_DIVIDE:
				if(parser->element.parse_divide(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
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

		if(left != NULL && IsAbstractTreeLeafToken(left->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id + 1);
		}
		if(right != NULL && IsAbstractTreeLeafToken(right->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id - 1);
		}
		ScriptParserElementNewLeafBinary(&parser->element, parent, left, right);
		parent->token = parser->element.tokens[token_id];
		if(parent->left != NULL)
		{
			PointerArrayPush(&parser->new_parent, parent->left);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left,
			GetLeftTokenID(parser->element.tokens, token_id - 1, CALCULATE_PRIORITY_MULTI_DIVIDE)) == FALSE)
		{
			return FALSE;
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->right, GetRightTokenID(parser->element.tokens, token_id, parser->element.num_tokens)) == FALSE)
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
 ScriptBasicParserParseDivide関数
 デフォルトの構文解析器で割り算を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseDivide(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_DIVIDE
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

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
				ABSTRACT_SYNTAX_TREE *new_parent;
			case TOKEN_TYPE_PLUS:
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MULTI:
				if(parser->element.parse_multi(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_DIVIDE:
				if(parser->element.parse_divide(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
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
		if(parent->left != NULL)
		{
			PointerArrayPush(&parser->new_parent, parent->left);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left,
			GetLeftTokenID(parser->element.tokens, token_id - 1, CALCULATE_PRIORITY_MULTI_DIVIDE)) == FALSE)
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
 ScriptBasicParserParsePlus関数
 デフォルトの構文解析器で足し算を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParsePlus(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_PLUS
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

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
				ABSTRACT_SYNTAX_TREE *new_parent;
			case TOKEN_TYPE_PLUS:
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
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
		if(parent->left != NULL)
		{
			PointerArrayPush(&parser->new_parent, parent->left);
		}
		if(left != NULL && IsAbstractTreeLeafToken(left->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id + 1);
		}
		if(right != NULL && IsAbstractTreeLeafToken(right->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id - 1);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left,
			GetLeftTokenID(parser->element.tokens, token_id - 1, CALCULATE_PRIORITY_PLUS_MINUS)) == FALSE)
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
 ScriptBasicParserParseMinus関数
 デフォルトの構文解析器で引き算を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseMinus(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_MINUS
		&& FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *left, *right;
		int next_id;

		FLAG_ON(parser->token_check_flag, token_id);

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
				ABSTRACT_SYNTAX_TREE *new_parent;
			case TOKEN_TYPE_PLUS:
				if(parser->element.parse_plus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
				}
				break;
			case TOKEN_TYPE_MINUS:
				if(parser->element.parse_minus(&parser->element, next_id, parent) == FALSE)
				{
					return FALSE;
				}
				if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
				{
					parent = new_parent;
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
		if(parent->left != NULL)
		{
			PointerArrayPush(&parser->new_parent, parent->left);
		}
		if(left != NULL && IsAbstractTreeLeafToken(left->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id + 1);
		}
		if(right != NULL && IsAbstractTreeLeafToken(right->token_type) != FALSE)
		{
			FLAG_ON(parser->token_check_flag, token_id - 1);
		}
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left,
			GetLeftTokenID(parser->element.tokens, token_id - 1, CALCULATE_PRIORITY_PLUS_MINUS)) == FALSE)
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
 ScriptBasicParserParseParen関数
 デフォルトの構文解析器で( )を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
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
					ABSTRACT_SYNTAX_TREE *new_parent;
					hierarchy--;
					if(hierarchy == 0)
					{
						if(ScriptBasicParserParseOneStepRecursive(parser, parent, i + 1) == FALSE)
						{
							return FALSE;
						}
						if((new_parent = (ABSTRACT_SYNTAX_TREE*)PointerArrayPop(&parser->new_parent)) != NULL)
						{
							parent = new_parent;
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
 ScriptBasicParserParseBrace関数
 デフォルトの構文解析器で{ }を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseBrace(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_LEFT_BRACE)
		{
			TOKEN *start_token = parser->element.tokens[token_id];
			int hierarchy = 0;
			FLAG_ON(parser->token_check_flag, token_id);
			while(token_id < parser->element.num_tokens)
			{
				token_id++;
				if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_RIGHT_BRACE)
				{
					if(hierarchy <= 0)
					{
						if(parent == NULL)
						{
							parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
							parent->left = parent->center = parent->right = NULL;
							parent->token = start_token;
							PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
							return TRUE;
						}
						else
						{
							parser->element.rule->error_message(parser->element.rule->error_message_data,
								parser->element.rule->file_names[start_token->file_id], start_token->line, "\"{\" is detected bad place.\n");
							return FALSE;
						}
					}
					hierarchy--;
				}
				else if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_LEFT_BRACE)
				{
					hierarchy++;
				}
			}
			parser->element.rule->error_message(parser->element.rule->error_message_data,
				parser->element.rule->file_names[start_token->file_id], start_token->line, "\"}\" is missed...\n");
			return FALSE;
		}
		else if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_RIGHT_BRACE)
		{
			FLAG_ON(parser->token_check_flag, token_id);
			if(parent == NULL)
			{
				parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
				parent->left = parent->center = parent->right = NULL;
				parent->token = parser->element.tokens[token_id];
				PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
				return TRUE;
			}
			else
			{
				TOKEN *token = parser->element.tokens[token_id];
				parser->element.rule->error_message(parser->element.rule->error_message_data,
					parser->element.rule->file_names[token->file_id], token->line, "\"}\" is detected bad place.\n");
			}
		}
	}

	return TRUE;
}

/*
 ScriptBasicParserParseFunction関数
 デフォルトの構文解析器で関数を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int ScriptBasicParserParseFunction(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	TOKEN *token = parser->element.tokens[token_id];
	if(FLAG_CHECK(parser->token_check_flag, token_id) == FALSE
		&& token->token_type == TOKEN_TYPE_IDENT)
	{
		TOKEN *left, *right;
		uintptr_t function_id;

		left = (token_id == 0) ? NULL : parser->element.tokens[token_id - 1];
		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];

		if((function_id = (uintptr_t)StringHashTableGet(parser->element.user_functions, token->name)) != 0)
		{
			FLAG_ON(parser->token_check_flag, token_id);
			token->token_type = SCRIPT_BASIC_RESERVED_USER_FUNCTION;

			if(parent == NULL)
			{
				parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
				PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
			}
			parent->token = token;
			parent->left = (ABSTRACT_SYNTAX_TREE*)function_id;
			parent->center = (ABSTRACT_SYNTAX_TREE*)(((parser->flags & SCRIPT_BASIC_PARSER_FLAG_WASTE_RETURN) == FALSE) ? FALSE : TRUE);
			if(right != NULL)
			{
				if(right->token_type == TOKEN_TYPE_LEFT_PAREN)
				{
					int start_id;
					int hierarchy = 0;
					int have_arg = FALSE;
					int num_arg = 0;
					int i;

					FLAG_ON(parser->token_check_flag, token_id);
					token_id++;
					FLAG_ON(parser->token_check_flag, token_id);
					token_id++;
					start_id = token_id;

					while(token_id < parser->element.num_tokens &&
						(!(hierarchy <= 0 && parser->element.tokens[token_id]->token_type == TOKEN_TYPE_RIGHT_PAREN)))
					{
						if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_LEFT_PAREN)
						{
							hierarchy++;
						}
						else if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_RIGHT_PAREN)
						{
							hierarchy--;
						}
						if(parser->element.tokens[token_id]->token_type != TOKEN_TYPE_COMMA)
						{
							have_arg = TRUE;
						}
						else if(hierarchy <= 0)
						{
							if(have_arg == FALSE)
							{
								const char *file_name = parser->element.rule->file_names[parser->element.tokens[token_id]->file_id];
								int line = parser->element.tokens[token_id]->line;
								parser->element.rule->error_message(parser->element.rule->error_message_data,
									file_name, line, "\",\" is bad placed.\n");
								return FALSE;
							}

							num_arg++;
							have_arg = FALSE;
						}
						token_id++;
					}
					if(token_id >= parser->element.num_tokens)
					{
						const char *file_name = parser->element.rule->file_names[parser->element.tokens[parser->element.num_tokens - 1]->file_id];
						parser->element.rule->error_message(parser->element.rule->error_message_data,
							file_name, 0, "\")\" is not found.\n");
						return FALSE;
					}
					if(have_arg != FALSE)
					{
						num_arg++;
					}
					parent->right = (ABSTRACT_SYNTAX_TREE*)num_arg;

					token_id = start_id;
					for(i=0; i<num_arg; i++)
					{
						TOKEN *right = (token_id < parser->element.num_tokens - 1) ? parser->element.tokens[token_id + 1] : NULL;
						token = parser->element.tokens[token_id];
						if(token->token_type == TOKEN_TYPE_IDENT
							&& StringHashTableGet(parser->element.user_functions, token->name) != NULL)
						{
							if(ScriptBasicParserParseFunction(parser, token_id, NULL) == FALSE)
							{
								return FALSE;
							}
						}
						else if(right != NULL && IsAbstractTreeLeafToken(token->token_type) != FALSE
							&& (right->token_type == TOKEN_TYPE_COMMA || right->token_type == TOKEN_TYPE_RIGHT_PAREN))
						{
							ABSTRACT_SYNTAX_TREE *tree = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*tree));
							tree->left = tree->center = tree->right = NULL;
							tree->token = parser->element.tokens[token_id];
							PointerArrayAppend(&parser->element.abstract_syntax_tree, tree);
							FLAG_ON(parser->token_check_flag, token_id);
						}
						else if(ScriptBasicParserParseOneStepRecursive(parser, NULL, token_id + 1) == FALSE)
						{
							return FALSE;
						}
						while(FLAG_CHECK(parser->token_check_flag, token_id) != FALSE)
						{
							token_id++;
						}
						if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_COMMA)
						{
							FLAG_ON(parser->token_check_flag, token_id);
							token_id++;
						}
					}
					if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_RIGHT_PAREN)
					{
						FLAG_ON(parser->token_check_flag, token_id);
						token_id++;
					}
				}
			}
		}
	}

	return TRUE;
}

/*
 ReleaseScriptBasicParser関数
 デフォルトの構文解析器のメモリを開放
 引数
 parser	: 構文解析器
*/
static void ReleaseScriptBasicParser(SCRIPT_BASIC_PARSER* parser)
{
	MEM_FREE_FUNC(parser->token_check_flag);
	ReleasePointerArray(&parser->element.abstract_syntax_tree);
	ReleasePointerArray(&parser->new_parent);
}

/*
 InitializeScriptBasicParser関数
 デフォルトの構文解析器を初期化
 引数
 parser				: 構文解析器
 rule				: 構文解析に使用する構文チェック用データ
 tokens				: 字句解析によって得られたトークン配列
 num_tokens			: 字句解析によって得られたトークンの数
 user_functions		: ユーザー定義関数の名前ハッシュテーブル
*/
void InitializeScriptBasicParser(
	SCRIPT_BASIC_PARSER* parser,
	SCRIPT_RULE_ELEMENT* rule,
	TOKEN** tokens,
	int num_tokens,
	STRING_HASH_TABLE* user_functions
)
{
#define TREE_BLOCK_SIZE 1024
#define PARENT_STACK_SIZE 128
	(void)memset(parser, 0, sizeof(*parser));
	parser->token_check_flag = (unsigned int*)MEM_ALLOC_FUNC((num_tokens + sizeof(*parser->token_check_flag) * 8 - 1) / (sizeof(*parser->token_check_flag) * 8) * sizeof(*parser->token_check_flag));
	(void)memset(parser->token_check_flag, 0, (num_tokens + sizeof(*parser->token_check_flag) * 8 - 1) / (sizeof(*parser->token_check_flag) * 8) * sizeof(*parser->token_check_flag));
	InitializePointerArray(&parser->element.abstract_syntax_tree, TREE_BLOCK_SIZE, NULL);
	parser->element.rule = rule;
	parser->element.memory_pool = rule->memory_pool;
	parser->element.user_functions = user_functions;

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
	parser->element.parse_brace = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseBrace;
	parser->element.parse_function = (int (*)(SCRIPT_PARSER_ELEMENT*, int, ABSTRACT_SYNTAX_TREE*))ScriptBasicParserParseFunction;
	parser->element.release = (void (*)(SCRIPT_PARSER_ELEMENT*))ReleaseScriptBasicParser;

	InitializePointerArray(&parser->new_parent, PARENT_STACK_SIZE, NULL);
#undef TREE_BLOCK_SIZE
}

/*
 ScriptBasicParserParseIf関数
 デフォルトの構文解析器でif制御構文を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
int ScriptBasicParserParseIf(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	int have_else;

	if(parent != NULL)
	{
		TOKEN *token = parser->element.tokens[token_id];
		parser->element.rule->error_message(parser->element.rule->error_message_data,
			parser->element.rule->file_names[token->file_id], token->line, "\"if\" is detected bad place.\n");
		return FALSE;
	}

	if(FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		FLAG_ON(parser->token_check_flag, token_id);
		parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
		parent->token = parser->element.tokens[token_id];
		parent->left = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent->left));
		(void)memset(parent->left, 0, sizeof(*parent->left));
		parent->center = parent->right = NULL;

		token_id++;
		if(parser->element.tokens[token_id]->token_type != TOKEN_TYPE_LEFT_PAREN)
		{
			TOKEN *token = parser->element.tokens[token_id];
			parser->element.rule->error_message(parser->element.rule->error_message_data,
				parser->element.rule->file_names[token->file_id], token->line, "\"(\" is missed after \"if\".\n");
			return FALSE;
		}
		FLAG_ON(parser->token_check_flag, token_id);
		token_id++;
		PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, token_id + 1) == FALSE)
		{
			return FALSE;
		}
		while(FLAG_CHECK(parser->token_check_flag, token_id) != FALSE)
		{
			token_id++;
		}
		if(parser->element.tokens[token_id]->token_type != TOKEN_TYPE_RIGHT_PAREN)
		{
			TOKEN *token = parser->element.tokens[token_id];
			parser->element.rule->error_message(parser->element.rule->error_message_data,
				parser->element.rule->file_names[token->file_id], token->line, "\")\" is missed after \"if\".\n");
			return FALSE;
		}
		FLAG_ON(parser->token_check_flag, token_id);
		token_id++;

		do
		{
			ABSTRACT_SYNTAX_TREE *else_tree = NULL;
			ABSTRACT_SYNTAX_TREE *brace_tree = NULL;
			have_else = FALSE;
			if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_LEFT_BRACE)
			{
				parser->element.parse_brace(&parser->element, token_id, NULL);
				token_id++;
				while(token_id < parser->element.num_tokens
					&& parser->element.tokens[token_id]->token_type != TOKEN_TYPE_RIGHT_BRACE)
				{
					int before_id = token_id;
					parser->flags |= SCRIPT_BASIC_PARSER_FLAG_WASTE_RETURN;
					if(parser->element.parse_function(&parser->element, token_id, NULL) == FALSE)
					{
						return FALSE;
					}
					parser->flags &= ~(SCRIPT_BASIC_PARSER_FLAG_WASTE_RETURN);
					while(FLAG_CHECK(parser->token_check_flag, token_id) != FALSE)
					{
						token_id++;
					}
					if(before_id == token_id)
					{
						if(parser->element.tokens[token_id]->token_type >= NUM_DEFAULT_TOKEN_TYPE)
						{
							int reserved_id = parser->element.tokens[token_id]->token_type - NUM_DEFAULT_TOKEN_TYPE;
							if(reserved_id >= 0 && reserved_id < parser->element.rule->num_reserved_rule)
							{
								if(parser->element.parse_reserved[reserved_id](&parser->element, token_id, NULL) == FALSE)
								{
									return FALSE;
								}
							}
						}
						else if(ScriptBasicParserParseOneStepRecursive(parser, NULL, token_id) == FALSE)
						{
							return FALSE;
						}
						FLAG_ON(parser->token_check_flag, token_id);
						while(FLAG_CHECK(parser->token_check_flag, token_id) != FALSE)
						{
							token_id++;
						}
					}
				}
				brace_tree = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*brace_tree));
				brace_tree->token = parser->element.tokens[token_id];
				brace_tree->left = brace_tree->center = brace_tree->right = NULL;
				PointerArrayAppend(&parser->element.abstract_syntax_tree, brace_tree);
				FLAG_ON(parser->token_check_flag, token_id);
				token_id++;
			}
			else
			{
				if(ScriptBasicParserParseOneStepRecursive(parser, NULL, token_id) == FALSE)
				{
					return FALSE;
				}
				while(FLAG_CHECK(parser->token_check_flag,token_id) != FALSE)
				{
					token_id++;
				}
				if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_SEMI_COLON)
				{
					token_id++;
				}
			}

			if(parser->element.tokens[token_id]->token_type == SCRIPT_BASIC_RESERVED_ELSE)
			{
				have_else = TRUE;
				else_tree = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*else_tree));
				else_tree->token = parser->element.tokens[token_id];
				else_tree->left = else_tree->center = else_tree->right = NULL;
				PointerArrayAppend(&parser->element.abstract_syntax_tree, else_tree);
				FLAG_ON(parser->token_check_flag, token_id);
				token_id++;
				if(parser->element.tokens[token_id]->token_type == SCRIPT_BASIC_RESERVED_IF)
				{
					have_else = FALSE;	// 再起呼び出し側でelse文は処理されるのでフラグOFF
					ScriptBasicParserParseIf(parser, token_id, NULL);
					token_id++;
					while(FLAG_CHECK(parser->token_check_flag, token_id) != FALSE)
					{
						token_id++;
					}
				}
			}
		} while(have_else != FALSE);
	}

	return TRUE;
}

/*
 ScriptBasicParserParseElse関数
 デフォルトの構文解析器でelse制御構文を解析
 この関数が呼び出される時点でエラー
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	常にFALSE
*/
int ScriptBasicParserParseElse(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	const char *file_name = parser->element.rule->file_names[parser->element.tokens[token_id]->file_id];
	int line = parser->element.tokens[token_id]->line;
	parser->element.rule->error_message(parser->element.rule->error_message_data,
		file_name, line, "\"else\" is bad placed.\n");
	return FALSE;
}

/*
 ScriptBasicParserParseWhile関数
 デフォルトの構文解析器でwhile制御構文を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
int ScriptBasicParserParseWhile(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parent != NULL)
	{
		TOKEN *token = parser->element.tokens[token_id];
		parser->element.rule->error_message(parser->element.rule->error_message_data,
			parser->element.rule->file_names[token->file_id], token->line, "\"while\" is detected bad place.\n");
		return FALSE;
	}

	if(FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		ABSTRACT_SYNTAX_TREE *brace_tree = NULL;
		int next_id;
		FLAG_ON(parser->token_check_flag, token_id);
		parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
		parent->token = parser->element.tokens[token_id];
		parent->left = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent->left));
		(void)memset(parent->left, 0, sizeof(*parent->left));
		parent->center = parent->right = NULL;

		token_id++;
		if(parser->element.tokens[token_id]->token_type != TOKEN_TYPE_LEFT_PAREN)
		{
			TOKEN *token = parser->element.tokens[token_id];
			parser->element.rule->error_message(parser->element.rule->error_message_data,
				parser->element.rule->file_names[token->file_id], token->line, "\"(\" is missed after \"while\".\n");
			return FALSE;
		}
		FLAG_ON(parser->token_check_flag, token_id);
		token_id++;
		PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		next_id = parser->element.tokens[token_id + 1]->token_type == TOKEN_TYPE_RIGHT_PAREN ? token_id : token_id + 1;
		if(ScriptBasicParserParseOneStepRecursive(parser, parent->left, next_id) == FALSE)
		{
			return FALSE;
		}
		while(FLAG_CHECK(parser->token_check_flag, token_id) != FALSE)
		{
			token_id++;
		}
		if(token_id == next_id && IsAbstractTreeLeafToken(parser->element.tokens[token_id]->token_type) != FALSE)
		{
			parent->left->token = parser->element.tokens[token_id];
			token_id++;
		}
		if(parser->element.tokens[token_id]->token_type != TOKEN_TYPE_RIGHT_PAREN)
		{
			TOKEN *token = parser->element.tokens[token_id];
			parser->element.rule->error_message(parser->element.rule->error_message_data,
				parser->element.rule->file_names[token->file_id], token->line, "\")\" is missed after \"while\".\n");
			return FALSE;
		}
		FLAG_ON(parser->token_check_flag, token_id);
		token_id++;

		if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_LEFT_BRACE)
		{
			parser->element.parse_brace(&parser->element, token_id, NULL);
			token_id++;
			while(token_id < parser->element.num_tokens
				&& parser->element.tokens[token_id]->token_type != TOKEN_TYPE_RIGHT_BRACE)
			{
				int before_id = token_id;
				parser->flags |= SCRIPT_BASIC_PARSER_FLAG_WASTE_RETURN;
				if(parser->element.parse_function(&parser->element, token_id, NULL) == FALSE)
				{
					return FALSE;
				}
				parser->flags &= ~(SCRIPT_BASIC_PARSER_FLAG_WASTE_RETURN);
				while(FLAG_CHECK(parser->token_check_flag, token_id) != FALSE)
				{
					token_id++;
				}
				if(before_id == token_id)
				{
					if(parser->element.tokens[token_id]->token_type >= NUM_DEFAULT_TOKEN_TYPE)
					{
						int reserved_id = parser->element.tokens[token_id]->token_type - NUM_DEFAULT_TOKEN_TYPE;
						if(reserved_id >= 0 && reserved_id < parser->element.rule->num_reserved_rule)
						{
							if(parser->element.parse_reserved[reserved_id](&parser->element, token_id, NULL) == FALSE)
							{
								return FALSE;
							}
						}
					}
					else if(ScriptBasicParserParseOneStepRecursive(parser, NULL, token_id) == FALSE)
					{
						return FALSE;
					}
					FLAG_ON(parser->token_check_flag, token_id);
					while(FLAG_CHECK(parser->token_check_flag, token_id) != FALSE)
					{
						token_id++;
					}
				}
			}
			brace_tree = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*brace_tree));
			brace_tree->token = parser->element.tokens[token_id];
			brace_tree->left = brace_tree->center = brace_tree->right = NULL;
			PointerArrayAppend(&parser->element.abstract_syntax_tree, brace_tree);
			FLAG_ON(parser->token_check_flag, token_id);
			token_id++;
		}
		else
		{
			if(ScriptBasicParserParseOneStepRecursive(parser, NULL, token_id) == FALSE)
			{
				return FALSE;
			}
			while(FLAG_CHECK(parser->token_check_flag, token_id) != FALSE)
			{
				token_id++;
			}
			if(parser->element.tokens[token_id]->token_type == TOKEN_TYPE_SEMI_COLON)
			{
				token_id++;
			}
		}
	}

	return TRUE;
}

/*
 ScriptBasicParserParseBreak関数
 デフォルトの構文解析器でbreak制御構文を解析
 引数
 parser		: 構文解析器
 token_id	: 解析中のトークンID
 parent		: 抽象構文木の親ノード
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
int ScriptBasicParserParseBreak(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
{
	if(parent != NULL)
	{
		TOKEN *token = parser->element.tokens[token_id];
		parser->element.rule->error_message(parser->element.rule->error_message_data,
			parser->element.rule->file_names[token->file_id], token->line, "\"break\" is detected bad place.\n");
		return FALSE;
	}

	if(FLAG_CHECK(parser->token_check_flag, token_id) == 0)
	{
		TOKEN *right;

		right = (token_id >= parser->element.num_tokens - 1) ? NULL : parser->element.tokens[token_id + 1];
		if(right == NULL || right->token_type != TOKEN_TYPE_SEMI_COLON)
		{
			TOKEN *token = parser->element.tokens[token_id];
			parser->element.rule->error_message(parser->element.rule->error_message_data,
				parser->element.rule->file_names[token->file_id], token->line, "\";\" is not found after break.\n");
		}

		FLAG_ON(parser->token_check_flag, token_id);
		parent = (ABSTRACT_SYNTAX_TREE*)MemoryPoolAllocate(parser->element.memory_pool, sizeof(*parent));
		parent->token = parser->element.tokens[token_id];
		parent->left = parent->center = parent->right = NULL;
		PointerArrayAppend(&parser->element.abstract_syntax_tree, parent);
		FLAG_ON(parser->token_check_flag, token_id + 1);
	}

	return TRUE;
}

#ifdef __cplusplus
}
#endif
