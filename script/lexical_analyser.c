#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "lexical_analyser.h"

#define FILE_NAMES_BUFFER_SIZE 128	// �t�@�C�����ێ��o�b�t�@�̃T�C�Y
#define MEMORY_POOL_SIZE 4096		// �g�[�N����ێ����郁�����v�[���̃T�C�Y

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeLexicalAnalyzer�֐�
 �\�[�X�R�[�h���g�[�N���ɕ������邽�߂̃f�[�^��������
 ����
 analyser			: �\�[�X�R�[�h���g�[�N���ɕ������邽�߂̃f�[�^
 file_path			: �\�[�X�R�[�h�̃p�X
 error_message_func	: �G���[���b�Z�[�W�\���p�֐��|�C���^
 message_func_data	: �G���[���b�Z�[�W�\���p�֐��Ŏg���f�[�^
*/
void InitializeLexicalAnalyzer(
	LEXICAL_ANALYSER* analyser,
	const char* file_path,
	void (*error_message_func)(void*, const char*, ... ),
	void* message_func_data
)
{
	(void)memset(analyser, 0, sizeof(analyser));

	InitializeMemoryPool(&analyser->memory_pool, MEMORY_POOL_SIZE);
	InitializePointerArray(&analyser->file_names, FILE_NAMES_BUFFER_SIZE,
		(void (*)(void*))MEM_FREE_FUNC);
	InitializePointerArray(&analyser->tokens, MEMORY_POOL_SIZE / sizeof(void*), NULL);
	analyser->file_path = MEM_STRDUP_FUNC(file_path);
	analyser->error_message_func = (error_message_func != NULL) ?
		error_message_func : (void (*)(const char*))DefaultErrorMessage;
	analyser->error_message_func_data = message_func_data;
}

/*
 GetNextTokenString�֐�
 ���̃g�[�N���̕�������擾����
 ����
 analyser		: �\�[�X�R�[�h���g�[�N���ɕ������邽�߂̃f�[�^
 code			: ��͒��̃R�[�h������
 buffer			: ��������i�[����A�h���X
 line_count		: ���݂̍s���̃J�E���g����ϐ��̃A�h���X
 token_type		: �g�[�N���̃^�C�v��ۑ�����ϐ��̃A�h���X
 current_file	: ���݂̃t�@�C��
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
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
	int comment_level;	// �u/**/�v�̃R�����g�͊K�w���ɂ��邽�߁A�J�E���g����

	buffer[0] = '\0';

	// �X�y�[�X�A�^�u�A���s��ǂݔ�΂�
	while(**code == ' ' || **code == '\t' || **code == '\n' || **code == '\r')
	{
		if(**code == '\n')
		{	// ���s�Ȃ�s�����X�V
			(*line_count)++;
			(*code)++;
		}
		else if((*code)[0] == '/' && (*code)[1] == '/')
		{	// �u//�v�̃R�����g�Ȃ̂ŉ��s�܂œǂݔ�΂�
			*code += 2;
			do
			{
				*code = (char*)GetNextUtf8Character(*code);
			} while(**code != '\0' && **code != '\n');
			if(**code == '\n')
			{
				*code++;
				(*line_count)++;
			}
			else
			{
				return FALSE;
			}
		}
		else if((*code)[0] == '/' && (*code)[1] == '*')
		{
			comment_level = 1;
			do
			{
				if(**code == '\0')
				{
					analyser->error_message_func(analyser->error_message_func_data,
						"Code is end before comment end.\nfile:%s\tline:%d\n", current_file, *line_count);
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
		*token_type = TOKEN_TYPE_IDENT;

		next = (char*)GetNextUtf8Character(*code);
		do
		{
			*buffer = **code;
			(*code)++;
			buffer++;
		} while(*code < next);

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
	}
	else if(isdigit(**code) != FALSE)
	{
		int period_count = 0;
		*token_type = TOKEN_TYPE_CONST_DIGIT;

		*buffer = **code;
		(*code)++;
		buffer++;

		if(**code == 'x')
		{	// 16�i��
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
				&& isdigit(**code) == FALSE && **code != '.')
			{
				analyser->error_message_func(analyser->error_message_func_data,
					"Constant digit has wrong character.\nfile:%s\tline:%d\n", current_file, *line_count);
				return FALSE;
			}
			else if(**code == '.')
			{
				if(period_count > 0)
				{
					analyser->error_message_func(analyser->error_message_func_data,
						"Constant float has over two periods.\nfile:%s\tline:%d\n", current_file, *line_count);
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
					"Code is end before const string end.\nfile:%s\tline:%d\n", current_file, *line_count);
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
			*token_type = TOKEN_TYPE_LESS_EQUSL;
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
 AppendToken�֐�
 �g�[�N���f�[�^�����������m�ۂ��Ēǉ�����
 ����
 analyser	: �\�[�X�R�[�h���g�[�N���ɕ������邽�߂̃f�[�^
 token_type	: �ǉ�����g�[�N���̃^�C�v
 file_id	: ��͒��̃t�@�C����ID
 line_count	: ���݂̍s�ԍ�
 str		: �g�[�N���̕�����
*/
static void AppendToekn(
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
 LexicalAnayze�֐�
 �\�[�X�R�[�h���g�[�N���ɕ�������
 ����
 analyser	: �\�[�X�R�[�h���g�[�N���ɕ������邽�߂̃f�[�^
 open_func	: �\�[�X�R�[�h�f�[�^���J�����߂̊֐��|�C���^
 read_func	: �\�[�X�R�[�h�f�[�^��ǂݍ��ނ��߂̊֐��|�C���^
 seek_func	: �\�[�X�R�[�h�f�[�^���V�[�N���邽�߂̊֐��|�C���^
 tell_func	: �\�[�X�R�[�h�f�[�^�̃V�[�N�ʒu���擾���邽�߂̊֐��|�C���^
 close_func	: �\�[�X�R�[�h�f�[�^����邽�߂̊֐��|�C���^
 open_data	: �\�[�X�R�[�h���J���ۂɎg���O���f�[�^
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
*/
int LexicalAnalyze(
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
			"Failed to open %s.", analyser->file_path);
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
			MEM_FREE_FUNC(code);
			(void)close_func(src);
			return FALSE;
		}
		AppendToekn(analyser, token_type,
			0, line_count, str);
	}
	AppendToekn(analyser, TOKEN_TYPE_END_OF_CODE, 0, line_count, NULL);

	MEM_FREE_FUNC(code);
	(void)close_func(src);

	return TRUE;
}

/*
 ReleaseLexicalAnalyzer�֐�
 �\�[�X�R�[�h���g�[�N���ɕ������邽�߂̃f�[�^���J������
 ����
 analyser	: �\�[�X�R�[�h���g�[�N���ɕ������邽�߂̃f�[�^
*/
void ReleaseLexicalAnalyzer(LEXICAL_ANALYSER* analyser)
{
	MEM_FREE_FUNC(analyser->file_path);
	ReleaseMemoryPool(&analyser->memory_pool);
	ReleasePointerArray(&analyser->file_names);
	ReleasePointerArray(&analyser->tokens);
}

#ifdef __cplusplus
e}
#endif
