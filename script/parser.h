#ifndef _INCLUDED_PARSER_H_
#define _INCLUDED_PARSER_H_

#include "token.h"
#include "rule.h"
#include "../memory_pool.h"
#include "../utils.h"

/*
 SCRIPT_PARSER_ELEMENT構造体
 スクリプトの構文解析の基本データ
*/
typedef struct _SCRIPT_PARSER_ELEMENT
{
	POINTER_ARRAY abstract_syntax_tree;	// 抽象構文木
	SCRIPT_RULE_ELEMENT *rule;			// 構文が正しいかチェックするデータ
	MEMORY_POOL *memory_pool;			// メモリを管理するデータ
	TOKEN **tokens;						// 字句解析によって得られたトークン
	int num_tokens;						// 字句解析によって得られたトークンの数
	const char **user_functions;		// ユーザー定義関数の文字列
	int num_user_functions;				// ユーザー定義関数の数
	void *user_data;					// ユーザー定義関数で使う任意のデータ
	// 構文解析実施関数(失敗するとFALSEが返る)
	int (*parse)(struct _SCRIPT_PARSER_ELEMENT* parser);
	// 代入を構文解析する
	int (*parse_assign)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 論理和を構文解析する
	int (*parse_bool_or)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 論理積を構文解析する
	int (*parse_bool_and)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 同値比較を構文解析する
	int (*parse_equal)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 不同値比較を構文解析する
	int (*parse_not_equal)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 大小比較(未満)を構文解析する
	int (*parse_less)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 大小比較(以下)を構文解析する
	int (*parse_less_equal)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 大小比較(超越)を構文解析する
	int (*parse_greater)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 大小比較(以上)を構文解析する
	int (*parse_greater_equal)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 掛け算を構文解析する
	int (*parse_multi)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 割り算を構文解析する
	int (*parse_divide)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 足し算を構文解析する
	int (*parse_plus)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 引き算を構文解析する
	int (*parse_minus)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// ( )を構文解析する
	int (*parse_paren)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// { }を構文解析する
	int (*parse_brace)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// 制御構文及び組み込み関数を構文解析する
	int (**parse_reserved)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);
	// メモリ開放用の関数
	void (*release)(struct _SCRIPT_PARSER_ELEMENT* parser);
} SCRIPT_PARSER_ELEMENT;

/*
 SCRIPT_BASIC_PARSER構造体
 デフォルトのスクリプト構文解析器
*/
typedef struct _SCRIPT_BASIC_PARSER
{
	SCRIPT_PARSER_ELEMENT element;
	// 抽象構文木にトークンを追加したら立つフラグ
	unsigned int *token_check_flag;
	// 同じ計算優先度の演算が続いた時に親変更用
	POINTER_ARRAY new_parent;
} SCRIPT_BASIC_PARSER;

#ifdef __cplusplus
extern "C" {
#endif

/*
 ScriptParserElementSetReservedParseRule関数
 構文解析器に予約語及び組み込み関数の構文解析用のルールを追加する
 引数
 parser				: 構文解析器
 parse_functions	: 構文解析処理を行う関数ポインタ配列
*/
EXTERN void ScriptParserElementSetReservedParseRule(
	SCRIPT_PARSER_ELEMENT* parser,
	int (**parse_functions)(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
);

/*
 InitializeScriptBasicParser関数
 デフォルトの構文解析器を初期化
 引数
 parser				: 構文解析器
 rule				: 構文解析に使用する構文チェック用データ
 tokens				: 字句解析によって得られたトークン配列
 num_tokens			: 字句解析によって得られたトークンの数
 user_functions		: ユーザー定義関数の名前配列
 num_user_functions	: ユーザー定義関数の数
 user_data			: ユーザー定義関数で使用するユーザーデータ
*/
EXTERN void InitializeScriptBasicParser(
	SCRIPT_BASIC_PARSER* parser,
	SCRIPT_RULE_ELEMENT* rule,
	TOKEN** tokens,
	int num_tokens,
	const char** user_functions,
	int num_user_functions,
	void* user_data
);

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
int ScriptBasicParserParseIf(SCRIPT_BASIC_PARSER* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_PARSER_H_
