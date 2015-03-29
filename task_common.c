#include "task.h"
#include "game_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 TaskTop関数
 タスクリストの先頭で実行されるタスク
 画面のクリアと入力初期化を実行
*/
void TaskTop(TASK* task)
{
	// 画面をクリア
	glClear(GL_COLOR_BUFFER_BIT);

	// OpenGLの設定をリセット
	SetUpStateOfOpenGL(task->tasks->game_data);

	// 入力状態を更新
	UpdateInput(&task->tasks->game_data->input);
}

/*
 TaskBottom関数
 タスクリストの最後で実行されるタスク
 画面の更新を行う
*/
void TaskBottom(TASK* task)
{
	glutSwapBuffers();
}

#ifdef __cplusplus
}
#endif
