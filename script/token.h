#ifndef _INCLUDED_TOKEN_H_
#define _INCLUDED_TOKEN_H_

#include "../types.h"

/*
 eTOKEN_TYPE列挙体
 トークンの種類
*/
typedef enum _eTOKEN_TYPE
{
	TOKEN_TYPE_ASSIGN,			// 代入
	TOKEN_TYPE_PLUS,			// 足し算
	TOKEN_TYPE_MINUS,			// 引き算
	TOKEN_TYPE_MULTI,			// 掛け算
	TOKEN_TYPE_DIVIDE,			// 割り算
	TOKEN_TYPE_MOD,				// 余り
	TOKEN_TYPE_BOOL_AND,		// 論理積
	TOKEN_TYPE_BOOL_OR,			// 論理和
	TOKEN_TYPE_BOOL_NOT,		// 論理否定
	TOKEN_TYPE_BIT_AND,			// ビット積
	TOKEN_TYPE_BIT_OR,			// ビット和
	TOKEN_TYPE_BIT_NOT,			// ビット反転
	TOKEN_TYPE_BIT_XOR,			// ビット排他的論理和
	TOKEN_TYPE_EQUAL,			// 比較(等しい)
	TOKEN_TYPE_NOT_EQUAL,		// 比較(等しくない)
	TOKEN_TYPE_LESS,			// 比較(未満)
	TOKEN_TYPE_LESS_EQUAL,		// 比較(以下)
	TOKEN_TYPE_GREATER,			// 比較(大きい)
	TOKEN_TYPE_GREATER_EQUAL,	// 比較(以上)
	TOKEN_TYPE_INCREMENT,		// インクリメント
	TOKEN_TYPE_DECREMENT,		// デクリメント
	TOKEN_TYPE_CONST_DIGIT,		// 整数定数
	TOKEN_TYPE_CONST_FLOAT,		// 小数定数
	TOKEN_TYPE_CONST_STRING,	// 文字列定数
	TOKEN_TYPE_SEMI_COLON,		// セミコロン
	TOKEN_TYPE_COMMA,			// コンマ
	TOKEN_TYPE_LEFT_PAREN,		// 小括弧始め
	TOKEN_TYPE_RIGHT_PAREN,		// 小括弧終わり
	TOKEN_TYPE_LEFT_BRACE,		// 中括弧始め
	TOKEN_TYPE_RIGHT_BRACE,		// 中括弧終わり
	TOKEN_TYPE_LEFT_BRACKET,	// 大括弧始め
	TOKEN_TYPE_RIGHT_BRACKET,	// 大括弧終わり
	TOKEN_TYPE_IDENT,			// 変数の識別子
	TOKEN_TYPE_FUNCTION,		// 関数の識別子
	TOKEN_TYPE_END_OF_CODE,		// コードの終端
	NUM_DEFAULT_TOKEN_TYPE
} eTOKEN_TYPE;

/*
 TOKEN構造体
 トークン1つ分の情報
*/
typedef struct _TOKEN
{
	uint16 length;		// 文字列の長さ
	uint16 token_type;	// トークンのタイプ
	uint16 file_id;		// ファイルの識別ID
	uint16 line;		// 行数
	char *name;			// トークンの名前
} TOKEN;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeToken関数
 スクリプトの字句解析によって得られるトークンデータを初期化する
 引数
 token	: トークンデータ
 line	: トークンの存在する行数
*/
EXTERN void InitializeToken(TOKEN* token, int line);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TOKEN_H_
