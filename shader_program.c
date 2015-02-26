#include <stdio.h>
#include <stdarg.h>
#include "shader_program.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 AddShaderSource関数
 GLSLのプログラムオブジェクトにシェーダーオブジェクトを追加
 引数
 program_id	: プログラムオブジェクト
 source		: GLSLのシェーダーソースコード
 type		: シェーダーのタイプ(頂点シェーダー or フラグメントシェーダー)
 返り値
	成功:TRUE	失敗:FALSE
*/
static int AddShaderSource(GLuint program_id, const char* source, GLenum type)
{
	GLint compiled;		// ソースコードのコンパイルの成功/失敗
	GLuint shader;		// シェーダーオブジェクト
	
	// シェーダーオブジェクトを生成
	shader = glCreateShader(type);

	// 生成したシェーダーオブジェクトにソースコードを渡す
	glShaderSource(shader, 1, &source, NULL);
	// 渡したソースコードをコンパイル
	glCompileShader(shader);
	// コンパイルに成功したか確認
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	// compiledの中身が0ならコンパイル失敗
	if(compiled == 0)
	{
		GLint length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		if(length > 0)
		{
			char *message = (char*)malloc(length);
			glGetShaderInfoLog(shader, length, &length, message);
			(void)printf("%s\n", message);
			free(message);
		}
		glDeleteShader(shader);
		return FALSE;
	}

	// 
	glAttachShader(program_id, shader);
	glDeleteShader(shader);

	return TRUE;
}

/*
 GenerateGlslProgram関数
 GLSLのプログラムオブジェクトを生成し、シェーダーをコンパイルする
 引数
 vertex_shader_source	: 頂点シェーダーのソースコード文字列
 fragment_shader_source	: フラグメントシェーダーのソースコード文字列
 返り値
	生成したプログラムオブジェクトのID
*/
GLuint GenerateGlslProgram(
	const char* vertex_shader_source,
	const char* fragment_shader_source
)
{
	GLuint program_id;	// プログラムオブジェクトのID

	// プログラムオブジェクトの生成をOpenGLに依頼
	program_id = glCreateProgram();

	// 頂点シェーダーを追加
	if(AddShaderSource(program_id, vertex_shader_source, GL_VERTEX_SHADER) == FALSE)
	{
		glDeleteProgram(program_id);
		return 0;
	}

	// フラグメントシェーダーを追加
	if(fragment_shader_source != NULL)
	{
		if(AddShaderSource(program_id, fragment_shader_source, GL_FRAGMENT_SHADER) == FALSE)
		{
			glDeleteProgram(program_id);
			return 0;
		}
	}

	return program_id;
}

/*
 ReleaseShaderProgram関数
 シェーダープログラムを開放する
 引数
 program	: シェーダープログラムの基本データ
*/
void ReleaseShaderProgram(SHADER_PROGRAM_BASE* program)
{
	glDeleteProgram(program->program_id);
	program->program_id = 0;
}

/*
 ShaderProgramLink関数
 シェーダープログラムをリンクする
 引数
 program	: シェーダープログラムの基本データ
 返り値
	成功:TRUE	失敗:FALSE
*/
int ShaderProgramLink(SHADER_PROGRAM_BASE* program)
{
	GLint status;	// リンクの成功/失敗を受ける

	// リンク実行
	glLinkProgram(program->program_id);
	// リンクに成功したか調べる
	glGetProgramiv(program->program_id, GL_LINK_STATUS, &status);
	if(status == 0)
	{
		GLint length;
		glGetProgramiv(program->program_id, GL_INFO_LOG_LENGTH, &length);
		if(length > 0)
		{
			char *message = (char*)malloc(length);
			glGetProgramInfoLog(program->program_id, length, &length, message);
			(void)printf("%s\n", message);
			free(message);
		}
		ReleaseShaderProgram(program);
		return FALSE;
	}

	return TRUE;
}

/*
 InitializeShaderProgram関数
 1つ分のGLSLのプログラマブルシェーダー管理データを初期化
 引数
 program				: シェーダープログラムの基本データ
 vertex_shader_source	: 頂点シェーダーのソースコード文字列
 fragment_shader_source	: フラグメントシェーダーのソースコード文字列
 可変長引数				: 頂点シェーダー中のin修飾の変数名とそれに割り当てるID
							(const char*の名前, GLuintのkeyを交互に指定. 最後にNULLを指定)
 返り値
	成功:TRUE	失敗:FALSE
*/
int InitializeShaderProgram(
	SHADER_PROGRAM_BASE* program,
	const char* vertex_shader_source,
	const char* fragment_shader_source,
	const char* first_attribute,
	...
)
{
	const char *attribute_name;	// 入力変数名
	va_list list;				// 可変長引数のリスト

	(void)memset(program, 0, sizeof(*program));

	if((program->program_id = GenerateGlslProgram(
		vertex_shader_source, fragment_shader_source)) == 0)
	{
		return FALSE;
	}

	// 入力変数の名前とIDを結び付ける
	attribute_name = first_attribute;
	va_start(list, first_attribute);
	while(attribute_name != NULL)
	{
		GLuint key = va_arg(list, GLuint);
		glBindAttribLocation(program->program_id, key, attribute_name);
		attribute_name = va_arg(list, const char*);
	}

	// コンパイルしたシェーダーをリンク
	if(ShaderProgramLink(program) == FALSE)
	{
		ReleaseShaderProgram(program);
		return FALSE;
	}

	return TRUE;
}

#ifdef __cplusplus
}
#endif
