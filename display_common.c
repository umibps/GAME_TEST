#include "display.h"

/*
 SetUpStateOfOpenGL�֐�
 OpenGL�̏�Ԃ��f�t�H���g��Ԃɐݒ肷��
 ����
 game_data	: �Q�[���S�̂��Ǘ�����f�[�^
*/
void SetUpStateOfOpenGL(GAME_DATA* game_data)
{
	// ���u�����h�̗L����
	glEnable(GL_BLEND);

	// ���u�����h�̕��@
		// �V�����`�悳���|���S���̕s�����x�ɍ��킹�ĕ`�悷��
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
