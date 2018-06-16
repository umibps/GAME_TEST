#include <time.h>
#include "game_data.h"
#include "initialize.h"
#include "random.h"
#include "script/compile.h"
#include "script/executor.h"

const char* TokenString(TOKEN* token)
{
	switch(token->token_type)
	{
	case TOKEN_TYPE_ASSIGN:
		return "=";
	case TOKEN_TYPE_PLUS:
		return "+";
	case TOKEN_TYPE_MINUS:
		return "-";
	case TOKEN_TYPE_MULTI:
		return "*";
	case TOKEN_TYPE_DIVIDE:
		return "/";
	case TOKEN_TYPE_BOOL_AND:
		return "&&";
	case TOKEN_TYPE_BOOL_OR:
		return "||";
	case TOKEN_TYPE_EQUAL:
		return "==";
	case TOKEN_TYPE_NOT_EQUAL:
		return "!=";
	case TOKEN_TYPE_LESS:
		return "<";
	case TOKEN_TYPE_LESS_EQUAL:
		return "<=";
	case TOKEN_TYPE_GREATER:
		return ">";
	case TOKEN_TYPE_GREATER_EQUAL:
		return ">=";
	case TOKEN_TYPE_LEFT_BRACE:
		return "{";
	case TOKEN_TYPE_RIGHT_BRACE:
		return "}";
	case TOKEN_TYPE_CONST_DIGIT:
	case TOKEN_TYPE_CONST_FLOAT:
	case TOKEN_TYPE_CONST_STRING:
	case TOKEN_TYPE_IDENT:
	case SCRIPT_BASIC_RESERVED_USER_FUNCTION:
		return token->name;
	case SCRIPT_BASIC_RESERVED_IF:
		return "if";
	case SCRIPT_BASIC_RESERVED_ELSE:
		return "else";
	case SCRIPT_BASIC_RESERVED_WHILE:
		return "while";
	case SCRIPT_BASIC_RESERVED_BREAK:
		return "break";
	}
	return "";
}

static void AstTreeDiplayRecursive(ABSTRACT_SYNTAX_TREE* ast, int step)
{
	if(ast->token == NULL)
	{
		return;
	}
	printf("->%s", TokenString(ast->token));
	if(ast->token->token_type == SCRIPT_BASIC_RESERVED_USER_FUNCTION)
	{
		printf(", %p, %p\n", ast->left, ast->right);
		return;
	}

	if(ast->left != NULL)
	{
		AstTreeDiplayRecursive(ast->left, step+1);
	}
	else
	{
		putchar('\n');
	}

	if(ast->right != NULL)
	{
		int i;
		for(i=0; i<step; i++)
		{
			printf("   ");
		}
		printf("   ");
		AstTreeDiplayRecursive(ast->right, step+1);
	}
}

static void AstTreeDisplay(int num_ast, ABSTRACT_SYNTAX_TREE** ast)
{
	int i;
	for(i=0; i<num_ast; i++)
	{
		AstTreeDiplayRecursive(ast[i], 0);
	}
}

static SCRIPT_BASIC_ARGUMENT TestScriptPrint(
	SCRIPT_BASIC_EXECUTOR* executor,
	void* dummy,
	SCRIPT_BASIC_ARGUMENT* arg,
	int num_arg
)
{
	SCRIPT_BASIC_ARGUMENT result = {0};
	int i;

	for(i=0; i<num_arg; i++)
	{
		switch(arg[i].type)
		{
		case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
			printf("%d", arg[i].value.integer_value);
			break;
		case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
			printf("%lf", arg[i].value.float_value);
			break;
		case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
			{
				char buffer[4096] = {0};
				char *dst = buffer;
				const char *src = arg[i].value.string_value;
				const char *next;
				while(*src != '\0')
				{
					next = GetNextUtf8Character(src);
					if(*next == '\\')
					{
						if(*(next+1) == 'n')
						{
							*dst = '\n';
							dst++;
							next++;
						}
						else if(*(next+1) == 't')
						{
							*dst = '\t';
							dst++;
							next++;
						}
						else
						{
							*dst = *next;
							dst++;
						}
					}
					else
					{
						while(src < next)
						{
							*dst = *src;
							src++;
							dst++;
						}
					}
					src = next;
				}
				printf("%s", buffer);
			}
			break;
		}
	}

	return result;
}

static SCRIPT_BASIC_ARGUMENT TestScriptScan(
	SCRIPT_BASIC_EXECUTOR* executor,
	void* dummy,
	SCRIPT_BASIC_ARGUMENT* arg,
	int num_arg
)
{
	SCRIPT_BASIC_ARGUMENT result = {SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER, 0};
	scanf("%d", &result.value.integer_value);
	return result;
}

int main(int argc, char** argv)
{
	FILE *fp;
	SCRIPT_BASIC_COMPILER compiler;
	SCRIPT_BASIC_EXECUTOR executor;
	uint8 *code;
	const char *func_names[] = {"print", "scan"};
	script_user_function function[] = {TestScriptPrint, TestScriptScan};

	if((fp = fopen("test.bin", "rb")) == NULL)
	{
		return 1;
	}

	InitializeScriptBasicCompiler(&compiler,
		"test.txt",
		func_names,
		sizeof(func_names) / sizeof(*func_names),
		NULL,
		NULL,
		NULL
	);
	code = ScriptBasicCompilerCompile(&compiler, fopen,
		fread, fseek, ftell, fclose, fp);
	//AstTreeDisplay((int)compiler.parser.element.abstract_syntax_tree.num_data,
	//	(ABSTRACT_SYNTAX_TREE**)compiler.parser.element.abstract_syntax_tree.buffer);

	if(code == NULL)
	{
		return 1;
	}

	InitializeScriptBasicExecutor(&executor, code, ScriptBasicCompilerGetByteCodeSize(&compiler),
		function, sizeof(func_names) / sizeof(*func_names), NULL);
	ScriptBasicExecutorExecute(&executor);
	ReleaseScriptBasicCompiler(&compiler);
	ReleaseScriptBasicExecutor(&executor);

	(void)fclose(fp);

#ifdef _WIN32
	(void)system("pause");
#endif

	return 0;
}
