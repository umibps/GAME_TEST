#ifndef _INCLUDED_SHADER_PROGRAM_H_
#define _INCLUDED_SHADER_PROGRAM_H_

#include <GL/glew.h>
#include <GL/glut.h>
#include "types.h"
#include "vertex.h"

/*
 eSHADER_ATTRIBUTE列挙体
 頂点バッファの属性定数
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
 DRAW_SQUARE_PROGRAM構造体
 画像全体を使って描画する場合のデータ
*/
typedef struct _DRAW_SQUARE_PROGRAM
{
	SHADER_PROGRAM_BASE base_data;						// シェーダープログラムの基本データ
	VERTEX_BUFFER vertex_buffer;						// (0, 1)のみの頂点バッファ
	GLint position_uniform_location;					// 描画する座標を入れる
														// 頂点シェーダープログラムの変数識別ID
	GLint texture_size_uniform_location;				// テクスチャ画像のサイズを入れる
														// 頂点シェーダープログラムの変数識別ID
	GLint reverse_half_display_size_unifrom_location;	// 画像サイズと指定座標からOpenGLでの座標を計算する
														// 頂点シェーダープログラムの変数識別ID
	GLint display_height_uniform_location;				// 描画領域の高さを指定する
														// 頂点シェーダープログラムの変数識別ID
	GLint zoom_uniform_location;						// X方向及びY方向の拡大率を入れる
														// 頂点シェーダープログラムの変数識別ID
	GLint rotate_uniform_location;						// 画像の回転角を入れる
														// 頂点シェーダープログラムの変数識別ID
	GLint color_uniform_location;						// 画像の色を指定する
														// フラグメントシェーダープログラムの変数識別ID
	GLint texture_uniform_location;						// 使用するテクスチャユニットのIDを指定する
														// フラグメントシェーダープログラムの変数識別ID
} DRAW_SQUARE_PROGRAM;

/*
 DISPLAY_PROGRAMS構造体
 使用するシェーダープログラムの集合体
*/
typedef struct _DISPLAY_PROGRAMS
{
	DRAW_SQUARE_PROGRAM draw_square;
} DISPLAY_PROGRAMS;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeDisplayPrograms関数
 描画に使うシェーダーの全てを初期化する
 引数
 programs	: シェーダー全体を管理するデータ
*/
EXTERN void InitializeDisplayPrograms(DISPLAY_PROGRAMS* programs);

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

/*
 InitializeDrawSquareProgram関数
 画像全体を使って描画するシェーダープログラムの初期化
 引数
 program		: シェーダーを管理するデータ
 display_width	: 描画領域の幅
 display_height	: 描画領域の高さ
 返り値
	成功:TRUE	失敗:FALSE
*/
EXTERN int InitializeDrawSquareProgram(
	DRAW_SQUARE_PROGRAM* program,
	float display_width,
	float display_height
);

/*
 DrawSquareProgramSetUniformLocation関数
 シェーダーオブジェクトからuniform属性の変数のIDを取得する
 引数
 program	: シェーダーを管理するデータ
*/
EXTERN void DrawSquareProgramSetUniformLocation(DRAW_SQUARE_PROGRAM* program);

/*
 InitializeDisplayPrograms関数
 描画に使うシェーダーの全てを初期化する
 引数
 programs	: シェーダー全体を管理するデータ
*/
EXTERN void InitializeDisplayPrograms(DISPLAY_PROGRAMS* programs);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SHADER_PROGRAM_H_
