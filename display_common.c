#include "display.h"

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
}
