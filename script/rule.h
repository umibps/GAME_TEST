#ifndef _INCLUDED_RULE_H_
#define _INCLUDED_RULE_H_

#include "token.h"
#include "abstract_syntax_tree.h"
#include "../memory_pool.h"

/*
 SCRIPT_RULE_ELEMENT構造体
 スクリプトの構文解析ルールの基本データ
*/
typedef struct _SCRIPT_RULE_ELEMENT
{
	// 代入を処理するルール(失敗時FALSEを返す)
	int (*assign_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 足し算またはプラス符号を処理するルール(失敗時FALSEを返す)
	int (*plus_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 引き算またはマイナス符号を処理するルール(失敗時FALSEを返す)
	int (*minus_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 掛け算を処理するルール(失敗時FALSEを返す)
	int (*multi_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 割り算を処理するルール
	int (*divide_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 余り計算を処理するルール
	int (*mod_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 論理和を処理するルール(失敗時FALSEを返す)
	int (*bool_or_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 論理積を処理するルール(失敗時FALSEを返す)
	int (*bool_and_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 同値比較を処理するルール
	int (*equal_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 不同値比較を処理するルール
	int (*not_equal_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 値比較(未満)を処理するルール
	int (*less_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 値比較(以下)を処理するルール
	int (*less_equal_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 値比較(超越)を処理するルール
	int (*greater_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 値比較(以下)を処理するルール
	int (*greater_equal_rule)(struct _SCRIPT_RULE_ELEMENT* rule, TOKEN* left, TOKEN* right);
	// 制御構文を処理するルール関数配列とその数
	int (**reserved_rule)(struct _SCRIPT_RULE_ELEMENT* rule, struct _LEXICAL_ANALYSER* analyser, void* function_data);
	int num_reserved_rule;
	// エラー発生時に使用する関数ポインタ
	void (*error_message)(void* error_message_data, const char* file_name,
		int line, const char* message, ...);
	// エラーメッセージ表示に使用するデータ
	void *error_message_data;
	// スクリプトファイルのファイル名(複数ファイル)
	const char **file_names;
	// 組み込み関数処理用のデータ
	void *function_data;
	// メモリ管理用データ
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
 ScriptRuleElementSetReservedRule関数
 構文解析に予約語のルールを設定する
 引数
 element		: スクリプトの構文解析ルールの基本データ
 rules			: 予約語のルール処理関数
 num_rules		: ルール処理関数の数
 function_data	: ルール処理実行時に使用するデータ
*/
EXTERN void ScriptRuleElementSetReservedRule(
	SCRIPT_RULE_ELEMENT* element,
	int (**rules)(struct _SCRIPT_RULE_ELEMENT* rule, struct _LEXICAL_ANALYSER* analyser, void* function_data),
	int num_rules,
	void* function_data
);

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
EXTERN void InitializeScriptBasicRule(
	SCRIPT_BASIC_RULE* rule,
	void (*error_message)(void* error_message_data, const char* file_name, int line, const char* message, ...),
	void* error_message_data,
	const char** file_names,
	MEMORY_POOL* memory_pool
);

/*
 ScriptBasicDummyRule関数
 ダミールール
 引数
 rule			: 構文解析のルールを管理するデータ
 analyser		: 字句解析器
 function_data	: ダミー
 返り値
	常にTRUE
*/
int ScriptBasicDummyRule(struct _SCRIPT_RULE_ELEMENT* rule,struct _LEXICAL_ANALYSER* analyser,void* function_data);

/*
 ScriptBasicIfRule関数
 if制御構文ルール
 引数
 rule			: 構文解析のルールを管理するデータ
 analyser		: 字句解析器
 function_data	: ダミー
 返り値
	ルールを満足:TRUE	ルールを満たさない:FALSE
*/
int ScriptBasicIfRule(struct _SCRIPT_RULE_ELEMENT* rule, struct _LEXICAL_ANALYSER* analyser, void* function_data);

#define ScriptBasicElseRule ScriptBasicDummyRule

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_RULE_H_
