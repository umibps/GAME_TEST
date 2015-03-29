#include <GL/glut.h>
#include "configure.h"

/*
 TimerCallback関数
 一定時間後に呼び出される関数
 引数
 timer_id	: タイマー識別ID (使わない)
*/
void TimerCallback(int timer_id)
{
	// タイマーを設定し直す
	glutTimerFunc(1000 / FRAMES_PER_SECOND,
		(void(*)(int))TimerCallback, 0);

	// 再描画要求
	glutPostRedisplay();
}
