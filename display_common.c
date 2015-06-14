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
 ReshapeWindow�֐�
 �E�B���h�E�ɃT�C�Y���ύX���ꂽ���̃R�[���o�b�N�֐�
 ����
 width	: OpenGL���`�悷��̈�̕�
 height	: OpenGL���`�悷��̈�̍���
*/
void ReshapeWindow(int width, int height)
{
	DISPLAY_DATA *display_data = &GetGameData()->display_data;

	display_data->display_width = width;
	display_data->display_height = height;

	glViewport(0, 0, width, height);
}

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

	// �e�N�X�`���̗L����
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
}

/*
 InitializeDisplayData�֐�
 �`��Ɋ֘A����f�[�^������������
 ����
 display_data	: �`��Ɋ֘A����f�[�^
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
