// Visual Studio 2005以降では古いとされる関数を使用するので
// 警告が出ないようにする
#if defined _MSC_VER && _MSC_VER >= 1400
# define _CRT_SECURE_NO_DEPRECATE
# define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "lexical_analyser.h"
#include "script.h"

#define FILE_NAMES_BUFFER_SIZE 128	// ファイル名保持バッファのサイズ
#define MEMORY_POOL_SIZE 4096		// トークンを保持するメモリプールのサイズ

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeLexicalAnalyser関数
 ソースコードをトークンに分解するためのデータを初期化
 引数
 analyser			: ソースコードをトークンに分解するためのデータ
 file_path			: ソースコードのパス
 error_message_func	: エラーメッセージ表示用関数ポインタ
 message_func_data	: エラーメッセージ表示用関数で使うデータ
*/
void InitializeLexicalAnalyser(
	LEXICAL_ANALYSER* analyser,
	const char* file_path,
	void (*error_message_func)(void*, const char*, ... ),
	void* message_func_data
)
{
	(void)memset(analyser, 0, sizeof(*analyser));

	InitializeMemoryPool(&analyser->memory_pool, MEMORY_POOL_SIZE);
	InitializePointerArray(&analyser->file_names, FILE_NAMES_BUFFER_SIZE,
		(void (*)(void*))MEM_FREE_FUNC);
	InitializePointerArray(&analyser->tokens, MEMORY_POOL_SIZE / sizeof(void*), NULL);
	analyser->file_path = MEM_STRDUP_FUNC(file_path);
	analyser->error_message_func = (error_message_func != NULL) ?
		error_message_func : (void (*)(void*, const char*, int, const char*, ...))DefaultScriptErrorMessage;
	analyser->error_message_func_data = message_func_data;
}

/*
 GetNextTokenString関数
 次のトークンの文字列を取得する
 引数
 analyser		: ソースコードをトークンに分解するためのデータ
 code			: 解析中のコード文字列
 buffer			: 文字列を格納するアドレス
 line_count		: 現在の行数のカウントする変数のアドレス
 token_type		: トークンのタイプを保存する変数のアドレス
 current_file	: 現在のファイル
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
static int GetNextTokenString(
	LEXICAL_ANALYSER* analyser,
	char** code,
	char* buffer,
	int* line_count,
	eTOKEN_TYPE* token_type,
	const char* current_file
)
{
	int comment_level;	// 「/**/」のコメントは階層化にするため、カウントする

	buffer[0] = '\0';

	// スペース、タブ、改行を読み飛ばす
	while(**code == ' ' || **code == '\t' || **code == '\n' || **code == '\r'
		|| ((**code == '/') && ((*code)[1] == '/' || (*code)[1] == '*')))
	{
		if(**code == '\n')
		{	// 改行なら行数を更新
			(*line_count)++;
			(*code)++;
		}
		else if((*code)[0] == '/' && (*code)[1] == '/')
		{	// 「//」のコメントなので改行まで読み飛ばす
			*code += 2;
			do
			{
				*code = (char*)GetNextUtf8Character(*code);
			} while(**code != '\0' && **code != '\n');
			if(**code == '\n')
			{
				(*code)++;
				(*line_count)++;
			}
			else
			{
				return FALSE;
			}
		}
		else if((*code)[0] == '/' && (*code)[1] == '*')
		{
			(*code) += 2;
			comment_level = 1;
			do
			{
				if(**code == '\0')
				{
					analyser->error_message_func(analyser->error_message_func_data,
						current_file, *line_count, "Code is end before comment end.\n");
					return FALSE;
				}
				else if((*code)[0] == '*' && (*code)[1] == '/')
				{
					comment_level--;
				}
				else if((*code)[0] == '/' && (*code)[1] == '*')
				{
					comment_level++;
				}
				else if(**code == '\n')
				{
					(*line_count)++;
				}

				*code = (char*)GetNextUtf8Character(*code);
			} while(comment_level > 0);
			(*code) += 2;
		}
		else
		{
			*code = (char*)GetNextUtf8Character(*code);
		}
	}
	if(**code == '\0')
	{
		return FALSE;
	}

	if(isalpha(**code) != FALSE || **code == '_')
	{
		char *next;
		char *token_string;
		int i;
		*token_type = TOKEN_TYPE_IDENT;

		token_string = buffer;
		next = (char*)GetNextUtf8Character(*code);
		do
		{
			*buffer = **code;
			(*code)++;
			buffer++;
		} while(*code < next);
		*buffer = '\0';

		while(isalnum(**code) != FALSE || **code == '_')
		{
			next = (char*)GetNextUtf8Character(*code);
			do
			{
				*buffer = **code;
				buffer++;
				(*code)++;
			} while(*code < next);
			*buffer = '\0';
		}

		for(i=0; i<analyser->num_reserved; i++)
		{
			if(strcmp(token_string, analyser->reserved[i]) == 0)
			{
				*token_type = NUM_DEFAULT_TOKEN_TYPE + i;
				break;
			}
		}
	}
	else if(isdigit(**code) != FALSE)
	{
		int period_count = 0;
		*token_type = TOKEN_TYPE_CONST_DIGIT;

		*buffer = **code;
		(*code)++;
		buffer++;

		if(**code == 'x')
		{	// 16進数
			*buffer = **code;
			(*code)++;
			buffer++;
		}

		while(isdigit(**code) != FALSE || **code == '.')
		{
			*buffer = **code;
			(*code)++;
			buffer++;

			if(**code != ' ' && **code != '\n' && **code != '\r' && **code != '\0'
				&& isdigit(**code) == FALSE && **code != '.' && **code != ',')
			{
				analyser->error_message_func(analyser->error_message_func_data,
					current_file, *line_count, "Constant digit has wrong character.\n");
				return FALSE;
			}
			else if(**code == '.')
			{
				if(period_count > 0)
				{
					analyser->error_message_func(analyser->error_message_func_data,
						current_file, *line_count, "Constant float has over two periods.\n");
					return FALSE;
				}
				else
				{
					period_count++;
					*token_type = TOKEN_TYPE_CONST_FLOAT;
				}
			}
		}
		*buffer = '\0';
	}
	else if(**code == '\"')
	{
		char *next;

		*token_type = TOKEN_TYPE_CONST_STRING;

		(*code)++;
		while(**code != '\"' && **code != '\0')
		{
			next = (char*)GetNextUtf8Character(*code);
			if(**code == '\0')
			{
				analyser->error_message_func(analyser->error_message_func_data,
					current_file, *line_count, "Code is end before const string end.\n\n");
				return FALSE;
			}
			else if(**code == '\n')
			{
				(*line_count)++;
				(*code)++;
			}
			else if(**code == '\\')
			{
				if(*next == 'n')
				{
					*buffer = '\n';
					buffer++;
					*code += 2;
				}
				else if(*next == 'r')
				{
					*buffer = '\r';
					buffer++;
					*code += 2;
				}
				else if(*next == 't')
				{
					*buffer = '\t';
					buffer++;
					*code += 2;
				}
				else if(*next == 'a')
				{
					*buffer = '\a';
					buffer++;
					*code += 2;
				}
				else if(*next == 'b')
				{
					*buffer = '\b';
					buffer++;
					*code += 2;
				}
				else
				{
					(*code)++;
					do
					{
						*buffer = **code;
						buffer++;
						(*code)++;
					} while(*code < next);
				}
			}
			else
			{
				do
				{
					*buffer = **code;
					(*code)++;
					buffer++;
				} while(*code < next);
			}
		}
		*buffer = '\0';
		(*code)++;
	}
	else if(**code == '+')
	{
		if((*code)[1] == '+')
		{
			*token_type = TOKEN_TYPE_INCREMENT;
			*buffer = **code;
			buffer++;
			(*code)++;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		else
		{
			*token_type = TOKEN_TYPE_PLUS;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		*buffer = '\0';
	}
	else if(**code == '-')
	{
		if((*code)[1] == '-')
		{
			*token_type = TOKEN_TYPE_DECREMENT;
			*buffer = **code;
			buffer++;
			(*code)++;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		else
		{
			*token_type = TOKEN_TYPE_MINUS;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		*buffer = '\0';
	}
	else if(**code == '*')
	{
		*token_type = TOKEN_TYPE_MULTI;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}
	else if(**code == '/')
	{
		*token_type = TOKEN_TYPE_DIVIDE;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}
	else if(**code == '%')
	{
		*token_type = TOKEN_TYPE_MOD;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}
	else if(**code == '&')
	{
		if((*code)[1] == '&')
		{
			*token_type = TOKEN_TYPE_BOOL_AND;
			*buffer = **code;
			buffer++;
			(*code)++;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		else
		{
			*token_type = TOKEN_TYPE_BIT_AND;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		*buffer = '\0';
	}
	else if(**code == '|')
	{
		if((*code)[1] == '|')
		{
			*token_type = TOKEN_TYPE_BOOL_OR;
			*buffer = **code;
			buffer++;
			(*code)++;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		else
		{
			*token_type = TOKEN_TYPE_BIT_OR;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		*buffer = '\0';
	}
	else if(**code == '!')
	{
		if((*code)[1] == '=')
		{
			*token_type = TOKEN_TYPE_NOT_EQUAL;
			*buffer = **code;
			buffer++;
			(*code)++;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		else
		{
			*token_type = TOKEN_TYPE_BOOL_NOT;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		*buffer = '\0';
	}
	else if(**code == '~')
	{
		*token_type = TOKEN_TYPE_BIT_NOT;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}
	else if(**code == '^')
	{
		*token_type = TOKEN_TYPE_BIT_XOR;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}
	else if(**code == '=')
	{
		if((*code)[1] == '=')
		{
			*token_type = TOKEN_TYPE_EQUAL;
			*buffer = **code;
			buffer++;
			(*code)++;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		else
		{
			*token_type = TOKEN_TYPE_ASSIGN;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		*buffer = '\0';
	}
	else if(**code == '>')
	{
		if((*code)[1] == '=')
		{
			*token_type = TOKEN_TYPE_GREATER_EQUAL;
			*buffer = **code;
			buffer++;
			(*code)++;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		else
		{
			*token_type = TOKEN_TYPE_GREATER;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		*buffer = '\0';
	}
	else if(**code == '<')
	{
		if((*code)[1] == '=')
		{
			*token_type = TOKEN_TYPE_LESS_EQUAL;
			*buffer = **code;
			buffer++;
			(*code)++;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		else
		{
			*token_type = TOKEN_TYPE_LESS;
			*buffer = **code;
			buffer++;
			(*code)++;
		}
		*buffer = '\0';
	}
	else if(**code == ';')
	{
		*token_type = TOKEN_TYPE_SEMI_COLON;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}
	else if(**code == ',')
	{
		*token_type = TOKEN_TYPE_COMMA;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}
	else if(**code == '(')
	{
		*token_type = TOKEN_TYPE_LEFT_PAREN;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}
	else if(**code == ')')
	{
		*token_type = TOKEN_TYPE_RIGHT_PAREN;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}
	else if(**code == '{')
	{
		*token_type = TOKEN_TYPE_LEFT_BRACE;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}
	else if(**code == '}')
	{
		*token_type = TOKEN_TYPE_RIGHT_BRACE;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}
	else if(**code == '[')
	{
		*token_type = TOKEN_TYPE_LEFT_BRACKET;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}
	else if(**code == ']')
	{
		*token_type = TOKEN_TYPE_RIGHT_BRACKET;
		*buffer = **code;
		buffer++;
		(*code)++;
		*buffer = '\0';
	}

	return TRUE;
}

/*
 AppendToken関数
 トークンデータをメモリを確保して追加する
 引数
 analyser	: ソースコードをトークンに分解するためのデータ
 token_type	: 追加するトークンのタイプ
 file_id	: 解析中のファイルのID
 line_count	: 現在の行番号
 str		: トークンの文字列
*/
static void AppendToken(
	LEXICAL_ANALYSER* analyser,
	eTOKEN_TYPE token_type,
	int file_id,
	int line_count,
	const char* str
)
{
	TOKEN *token = NULL;

	if(token_type == TOKEN_TYPE_CONST_DIGIT
		|| token_type == TOKEN_TYPE_CONST_FLOAT
		|| token_type == TOKEN_TYPE_CONST_STRING
		|| token_type == TOKEN_TYPE_IDENT
	)
	{
		size_t length = strlen(str);

		token = MemoryPoolAllocate(&analyser->memory_pool,
			sizeof(*token) + length + 1);
		token->length = (uint16)length;
		token->token_type = (uint16)token_type;
		token->file_id = (uint16)file_id;
		token->line = (uint16)line_count;
		token->name = (char*)((uint8*)token + sizeof(*token));
		(void)strcpy(token->name, str);
	}
	else
	{
		token = MemoryPoolAllocate(&analyser->memory_pool,
			sizeof(*token));
		token->length = 0;
		token->token_type = (uint16)token_type;
		token->file_id = (uint16)file_id;
		token->line = (uint16)line_count;
		token->name = NULL;
	}

	PointerArrayAppend(&analyser->tokens, token);
}

/*
 LexicalAnayse関数
 ソースコードをトークンに分解する
 引数
 analyser	: ソースコードをトークンに分解するためのデータ
 open_func	: ソースコードデータを開くための関数ポインタ
 read_func	: ソースコードデータを読み込むための関数ポインタ
 seek_func	: ソースコードデータをシークするための関数ポインタ
 tell_func	: ソースコードデータのシーク位置を取得するための関数ポインタ
 close_func	: ソースコードデータを閉じるための関数ポインタ
 open_data	: ソースコードを開く際に使う外部データ
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
int LexicalAnalyse(
	LEXICAL_ANALYSER* analyser,
	void* (*open_func)(const char*, const char*, void*),
	size_t (*read_func)(void*, size_t, size_t, void*),
	void* (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	int (*close_func)(void*),
	void* open_data
)
{
	eTOKEN_TYPE token_type;
	char str[4096];
	void *src;
	long length;
	char *code;
	char *p;
	int line_count = 1;

	src = open_func(analyser->file_path, "rb", open_data);

	if(src == NULL)
	{
		analyser->error_message_func(analyser->error_message_func_data,
			analyser->file_path, 0, "Failed to open %s.", analyser->file_path);
		return FALSE;
	}

	PointerArrayAppend(&analyser->file_names, MEM_STRDUP_FUNC(analyser->file_path));

	(void)seek_func(src, 0, SEEK_END);
	length = tell_func(src);
	(void)seek_func(src, 0, SEEK_SET);

	code = (char*)MEM_ALLOC_FUNC(length+1);
	(void)read_func(code, 1, length, src);
	code[length] = '\0';

	p = code;
	while(*p != '\0')
	{
		if(GetNextTokenString(analyser, &p, str, &line_count, &token_type, analyser->file_path) == FALSE)
		{
			if(p != code + length)
			{
				MEM_FREE_FUNC(code);
				(void)close_func(src);
				return FALSE;
			}
			else
			{
				break;
			}
		}
		AppendToken(analyser, token_type,
			0, line_count, str);
	}
	AppendToken(analyser, TOKEN_TYPE_END_OF_CODE, 0, line_count, NULL);

	MEM_FREE_FUNC(code);
	(void)close_func(src);

	return TRUE;
}

/*
 ReleaseLexicalAnalyser関数
 ソースコードをトークンに分解するためのデータを開放する
 引数
 analyser	: ソースコードをトークンに分解するためのデータ
*/
void ReleaseLexicalAnalyser(LEXICAL_ANALYSER* analyser)
{
	MEM_FREE_FUNC(analyser->file_path);
	ReleaseMemoryPool(&analyser->memory_pool);
	ReleasePointerArray(&analyser->file_names);
	ReleasePointerArray(&analyser->tokens);
}

/*
 LexicalAnalyserReadToken関数
 構文解析時にトークンを1つ取得
 引数
 analyser	: ソースコードをトークンに分解するためのデータ
 返り値
	トークン
*/
TOKEN* LexicalAnalyserReadToken(LEXICAL_ANALYSER* analyser)
{
	TOKEN *ret = NULL;
	if(analyser->reference < (int)analyser->tokens.num_data)
	{
		ret = ((TOKEN**)analyser->tokens.buffer)[analyser->reference];
		analyser->reference++;
	}

	return ret;
}

/*
 LexicalAnalyserPeekToken関数
 構文解析時に現在の位置から
 指定個数先のトークンを取得する
 引数
 analyser	: ソースコードをトークンに分解するためのデータ
 id			: 飛ばす個数
 返り値
	トークン
*/
TOKEN* LexicalAnalyserPeekToken(LEXICAL_ANALYSER* analyser, int id)
{
	TOKEN *ret = NULL;
	int index = analyser->reference + id;
	if(index < (int)analyser->tokens.num_data)
	{
		ret = ((TOKEN**)analyser->tokens.buffer)[index];
	}

	return ret;
}

/*
 LexicalAnalyserSetReserved関数
 予約語を設定する
 引数
 analyser		: ソースコードをトークンに分解するためのデータ
 reserved		: 予約語の文字列データ
 reserved_ids	: 予約語の識別IDデータ
 num_reserved	: 設定する予約語の数
*/
void LexicalAnalyserSetReserved(
	LEXICAL_ANALYSER* analyser,
	const char** reserved,
	uint16* reserved_ids,
	int num_reserved
)
{
	uint8 *allocated_data;
	size_t allocate_size = 0;
	char *str;
	int i;

	allocate_size = sizeof(char*) * num_reserved;
	for(i=0; i<num_reserved; i++)
	{
		allocate_size += strlen(reserved[i]) + 1;
	}
	allocate_size += allocate_size % 4;
	analyser->reserved = (char**)(allocated_data = (uint8*)MemoryPoolAllocate(&analyser->memory_pool, allocate_size + sizeof(uint16) * num_reserved));
	str = (char*)&allocated_data[sizeof(char*) * num_reserved];
	analyser->reserved_type = (uint16*)&allocated_data[allocate_size + (allocate_size % 4)];

	for(i=0; i<num_reserved; i++)
	{
		analyser->reserved[i] = str;
		str += strlen(reserved[i]) + 1;
		(void)strcpy(analyser->reserved[i], reserved[i]);
		analyser->reserved_type[i] = reserved_ids[i];
	}
	analyser->num_reserved = num_reserved;
}

#ifdef __cplusplus
}
#endif
