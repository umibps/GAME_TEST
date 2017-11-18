#include <string.h>
#include "token.h"

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
void InitializeToken(TOKEN* token, int line)
{
	(void)memset(token, 0, sizeof(*token));
	token->line = (uint16)line;
}

#ifdef __cplusplus
}
#endif
