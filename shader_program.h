#ifndef _INCLUDED_SHADER_PROGRAM_H_
#define _INCLUDED_SHADER_PROGRAM_H_

#include <GL/glew.h>
#include <GL/glut.h>
#include "types.h"

/*
*/
typedef enum _eSHADER_ATTRIBUTE
{
	SHADER_ATTRIBUTE_VERTEX,
	SHADER_ATTRIBUTE_TEXTURE_COORD,
	SHADER_ATTRIBUTE_COLOR
} eSHADER_ATTRIBUTE;

/*
 SHADER_PROGRAM_BASE構造体
 GLSLの基本データ
*/
typedef struct _SHADER_PROGRAM_BASE
{
	GLuint program_id;
} SHADER_PROGRAM_BASE;

/*
*/
typedef struct _DRAW_SQUARE_PROGRAM
{
	SHADER_PROGRAM_BASE base_data;			// シェーダープログラムの基本データ
	GLint texture_size_uniform_location;	// テクスチャ画像のサイズを入れる
											// シェーダープログラムの変数識別ID
	GLint display_size_unifrom_location;	// 画面サイズを入れる
											// シェーダープログラムの変数識別ID
	GLint zoom_uniform_location;			// X方向及びY方向の拡大率を入れる
											// シェーダープログラムの変数識別ID
	GLint rotate_uniform_location;			// 画像の回転角を入れる
											// シェーダープログラムの変数識別ID
} DRAW_SQUARE_PROGRAM;

#ifdef __cplusplus
extern "C" {
#endif

/*
 GenerateGlslProgram関数
 GLSLのプログラムオブジェクトを生成し、シェーダーをコンパイルする
 引数
 vertex_shader_source	: 頂点シェーダーのソースコード文字列
 fragment_shader_source	: フラグメントシェーダーのソースコード文字列
 返り値
	生成したプログラムオブジェクトのID
*/
EXTERN GLuint GenerateGlslProgram(
	const char* vertex_shader_source,
	const char* fragment_shader_source
);

/*
 ReleaseShaderProgram関数
 シェーダープログラムを開放する
 引数
 program	: シェーダープログラムの基本データ
*/
EXTERN void ReleaseShaderProgram(SHADER_PROGRAM_BASE* program);

/*
 ShaderProgramLink関数
 シェーダープログラムをリンクする
 引数
 program	: シェーダープログラムの基本データ
 返り値
	成功:TRUE	失敗:FALSE
*/
EXTERN int ShaderProgramLink(SHADER_PROGRAM_BASE* program);

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
EXTERN int InitializeShaderProgram(
	SHADER_PROGRAM_BASE* program,
	const char* vertex_shader_source,
	const char* fragment_shader_source,
	const char* first_attribute,
	...
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SHADER_PROGRAM_H_
