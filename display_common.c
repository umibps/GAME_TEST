#include "display.h"
#include "game_data.h"

#ifdef __cplusplus
extern "C" {
#endif

void Display(void)
{
	GAME_DATA *game_data = GetGameData();
	
	ExecuteTasks(&game_data->tasks);
}

/*
 ReshapeWindow関数
 ウィンドウにサイズが変更された時のコールバック関数
 引数
 width	: OpenGLが描画する領域の幅
 height	: OpenGLが描画する領域の高さ
*/
void ReshapeWindow(int width, int height)
{
	DISPLAY_DATA *display_data = &GetGameData()->display_data;

	display_data->display_width = width;
	display_data->display_height = height;

	glViewport(0, 0, width, height);
}

/*
 SetUpStateOfOpenGL関数
 OpenGLの状態をデフォルト状態に設定する
 引数
 game_data	: ゲーム全体を管理するデータ
*/
void SetUpStateOfOpenGL(GAME_DATA* game_data)
{
	// αブレンドの有効化
	glEnable(GL_BLEND);

	// αブレンドの方法
		// 新しく描画されるポリゴンの不透明度に合わせて描画する
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// テクスチャの有効化
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
}

/*
 InitializeDisplayData関数
 描画に関連するデータを初期化する
 引数
 display_data	: 描画に関連するデータ
*/
void InitializeDisplayData(DISPLAY_DATA* display_data)
{
	InitializeDisplayPrograms(&display_data->programs);
	InitializePriorityArray(&display_data->draw_items,
		1024, (void (*)(void*))DeleteDrawItemBase);
	InitializeImageTextures(&display_data->textures);
	display_data->display_width = glutGet(GLUT_SCREEN_WIDTH);
	display_data->display_height = glutGet(GLUT_SCREEN_HEIGHT);
}

#ifdef __cplusplus
}
#endif
