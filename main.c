#include <time.h>
#include "game_data.h"
#include "initialize.h"
#include "random.h"
#include "script/compile.h"

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
	}
	return "";
}

static void AstTreeDiplayRecursive(ABSTRACT_SYNTAX_TREE* ast, int step)
{
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

int main(int argc, char** argv)
{
	FILE *fp;
	SCRIPT_BASIC_COMPILER compiler;
	const char *func_names[] = {"print"};

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
	ScriptBasicCompilerCompile(&compiler, fopen,
		fread, fseek, ftell, fclose, fp);
	AstTreeDisplay((int)compiler.parser.element.abstract_syntax_tree.num_data,
		(ABSTRACT_SYNTAX_TREE**)compiler.parser.element.abstract_syntax_tree.buffer);

	ReleaseScriptBasicCompiler(&compiler);

	(void)fclose(fp);

#ifdef _WIN32
	(void)system("pause");
#endif

	return 0;
}
