#include <GL/glut.h>
#include "configure.h"

/*
 TimerCallback�֐�
 ��莞�Ԍ�ɌĂяo�����֐�
 ����
 timer_id	: �^�C�}�[����ID (�g��Ȃ�)
*/
void TimerCallback(int timer_id)
{
	// �^�C�}�[��ݒ肵����
	glutTimerFunc(1000 / FRAMES_PER_SECOND,
		(void(*)(int))TimerCallback, 0);

	// �ĕ`��v��
	glutPostRedisplay();
}
