#include "task.h"
#include "game_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 TaskTop�֐�
 �^�X�N���X�g�̐擪�Ŏ��s�����^�X�N
 ��ʂ̃N���A�Ɠ��͏����������s
*/
void TaskTop(TASK* task)
{
	// ��ʂ��N���A
	glClear(GL_COLOR_BUFFER_BIT);

	// OpenGL�̐ݒ�����Z�b�g
	SetUpStateOfOpenGL(task->tasks->game_data);

	// ���͏�Ԃ��X�V
	UpdateInput(&task->tasks->game_data->input);
}

/*
 TaskBottom�֐�
 �^�X�N���X�g�̍Ō�Ŏ��s�����^�X�N
 ��ʂ̍X�V���s��
*/
void TaskBottom(TASK* task)
{
	glutSwapBuffers();
}

#ifdef __cplusplus
}
#endif
