#include "../game_data.h"
#include "../configure.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 KeyboardDownCallback�֐�
 �L�[�{�[�h�̃L�[�������ꂽ���̃R�[���o�b�N�֐�
 ����
 key	: �����ꂽ�L�[
 x		: �L�[�������ꂽ���̃}�E�X��X���W
 y		: �L�[�������ꂽ���̃}�E�X��Y���W
*/
void KeyboardDownCallback(unsigned char key, int x, int y)
{
	GAME_INPUT *input = &GetGameData()->input;
	DISPLAY_DATA *display_data = &GetGameData()->display_data;

	input->keyboard.keyboard[key/(sizeof(unsigned int)*8)]
		|= (1 << (key % (sizeof(unsigned int)*8)));
	input->mouse.x = x * DISPLAY_WIDTH / display_data->display_width;
	input->mouse.y = y * DISPLAY_HEIGHT / display_data->display_height;
}

/*
 KeyboardUpCallback�֐�
 �L�[�{�[�h�̃L�[�������ꂽ���̃R�[���o�b�N�֐�
 ����
 key	: �����ꂽ�L�[
 x		: �L�[�������ꂽ���̃}�E�X��X���W
 y		: �L�[�������ꂽ���̃}�E�X��Y���W
*/
void KeyboardUpCallback(unsigned char key, int x, int y)
{
	GAME_INPUT *input = &GetGameData()->input;
	DISPLAY_DATA *display_data = &GetGameData()->display_data;

	input->keyboard.keyboard[key/(sizeof(unsigned int)*8)]
		&= ~(1 << (key % (sizeof(unsigned int)*8)));
	input->mouse.x = x * DISPLAY_WIDTH / display_data->display_width;
	input->mouse.y = y * DISPLAY_HEIGHT / display_data->display_height;
}

/*
 KeyboardSpecialDownCallback�֐�
 �L�[�{�[�h�̓���L�[�������ꂽ���̃R�[���o�b�N�֐�
 ����
 key	: �����ꂽ����L�[
 x		: ����L�[�������ꂽ���̃}�E�X��X���W
 y		: ����L�[�������ꂽ���̃}�E�X��Y���W
*/
void KeyboardSpecialDownCallback(unsigned int key, int x, int y)
{
	GAME_INPUT *input = &GetGameData()->input;
	DISPLAY_DATA *display_data = &GetGameData()->display_data;
	
	if(key < 256)
	{
		input->keyboard.special[key/(sizeof(unsigned int)*8)]
			|= (1 << (key % (sizeof(unsigned int)*8)));
	}
	input->mouse.x = x * DISPLAY_WIDTH / display_data->display_width;
	input->mouse.y = y * DISPLAY_HEIGHT / display_data->display_height;
}

/*
 KeyboardSpecialUpCallback�֐�
 �L�[�{�[�h�̓���L�[�������ꂽ���̃R�[���o�b�N�֐�
 ����
 key	: �����ꂽ����L�[
 x		: ����L�[�������ꂽ���̃}�E�X��X���W
 y		: ����L�[�������ꂽ���̃}�E�X��Y���W
*/
void KeyboardSpecialUpCallback(unsigned int key, int x, int y)
{
	GAME_INPUT *input = &GetGameData()->input;
	DISPLAY_DATA *display_data = &GetGameData()->display_data;

	if(key < 256)
	{
		input->keyboard.special[key/(sizeof(unsigned int)*8)]
			&= ~(1 << (key % (sizeof(unsigned int)*8)));
	}
	input->mouse.x = x * DISPLAY_WIDTH / display_data->display_width;
	input->mouse.y = y * DISPLAY_HEIGHT / display_data->display_height;
}


/*
 JoystickCallback�֐�
 �W���C�X�e�B�b�N�̏�Ԃ��X�V���ꂽ���̃R�[���o�b�N�֐�
 ����
 button	: ��Ԃ��X�V���ꂽ�{�^��
 x		: �X�e�B�b�N��X���W
 y		: �X�e�B�b�N��Y���W
 z		: �X�e�B�b�N��Z���W
*/
void JoystickCallback(unsigned int button, int x, int y, int z)
{
#define STICK_THRESHOLD (300)	// �X�e�B�b�N�̔���臒l
	GAME_INPUT *input = &GetGameData()->input;
	
	input->joystick.button = button;

	input->joystick.stick = 0;
	if(x > STICK_THRESHOLD)
	{
		input->joystick.stick |= BUTTON_FLAG_RIGHT;
	}
	else
	{		
		input->joystick.stick &= ~(BUTTON_FLAG_RIGHT);
	}

	if(x < -STICK_THRESHOLD)
	{
		input->joystick.stick |= BUTTON_FLAG_LEFT;
	}
	else
	{		
		input->joystick.stick &= ~(BUTTON_FLAG_LEFT);
	}

	if(y > STICK_THRESHOLD)
	{
		input->joystick.stick |= BUTTON_FLAG_DOWN;
	}
	else
	{		
		input->joystick.stick &= ~(BUTTON_FLAG_DOWN);
	}

	if(y < - STICK_THRESHOLD)
	{
		input->joystick.stick |= BUTTON_FLAG_UP;
	}
	else
	{		
		input->joystick.stick &= ~(BUTTON_FLAG_UP);
	}
}

/*
 MouseCallback�֐�
 �}�E�X�̏�Ԃ��X�V���ꂽ���̃R�[���o�b�N�֐�
 ����
 button	: ��Ԃ̕ύX���������{�^��
 state	: �{�^���̏��
 x		: �}�E�X��X���W
 y		: �}�E�X��Y���W	
*/
void MouseCallback(int button, int state, int x, int y)
{
	GAME_INPUT *input = &GetGameData()->input;
	DISPLAY_DATA *display_data = &GetGameData()->display_data;

	if(state == GLUT_DOWN)
	{
		input->mouse.mouse[button/(sizeof(unsigned int)*8)]
			|= (1 << button % (sizeof(unsigned int)*8));
	}
	else if(state == GLUT_UP)
	{
		input->mouse.mouse[button/(sizeof(unsigned int)*8)]
			&= ~(1 << button % (sizeof(unsigned int)*8));
	}

	input->mouse.x = x * DISPLAY_WIDTH / display_data->display_width;
	input->mouse.y = y * DISPLAY_HEIGHT / display_data->display_height;
}

/*
 SetInputCallbacks�֐�
 ���͂ɑ΂���R�[���o�b�N�֐���ݒ肷��
*/
void SetInputCallbacks(void)
{
	glutKeyboardFunc((void (*)(unsigned char, int, int))KeyboardDownCallback);
	glutKeyboardUpFunc((void (*)(unsigned char, int, int))KeyboardUpCallback);
	glutSpecialFunc((void (*)(int, int, int))KeyboardSpecialDownCallback);
	glutSpecialUpFunc((void (*)(int, int, int))KeyboardSpecialUpCallback);
	glutMouseFunc((void (*)(int, int, int, int))MouseCallback);
	glutJoystickFunc((void (*)(unsigned int, int, int, int))JoystickCallback, 1000/60);
}

/*
 LoadDefaultInputSettings�֐�
 ���͐ݒ�̃f�t�H���g�f�[�^��ݒ肷��
 ����
 input	: ���͏����S�̂��Ǘ�����f�[�^
*/
void LoadDefaultInputSettings(GAME_INPUT* input)
{
	(void)memset(input->keyboard.keyboard_map, 0xFF, sizeof(input->keyboard.keyboard_map));
	input->keyboard.keyboard_map[BUTTON_ID_0] = 'z';
	input->keyboard.keyboard_map[BUTTON_ID_1] = 'x';
	
	(void)memset(input->keyboard.special_map, 0xFF, sizeof(input->keyboard.special_map));
	input->keyboard.special_map[BUTTON_ID_UP] = GLUT_KEY_UP;
	input->keyboard.special_map[BUTTON_ID_DOWN] = GLUT_KEY_DOWN;
	input->keyboard.special_map[BUTTON_ID_LEFT] = GLUT_KEY_LEFT;
	input->keyboard.special_map[BUTTON_ID_RIGHT] = GLUT_KEY_RIGHT;

	(void)memset(input->joystick.joystick_map, 0xFF, sizeof(input->joystick.joystick_map));
	input->joystick.joystick_map[BUTTON_ID_UP] = BUTTON_ID_UP;
	input->joystick.joystick_map[BUTTON_ID_DOWN] = BUTTON_ID_DOWN;
	input->joystick.joystick_map[BUTTON_ID_LEFT] = BUTTON_ID_LEFT;
	input->joystick.joystick_map[BUTTON_ID_RIGHT] = BUTTON_ID_RIGHT;
	input->joystick.joystick_map[BUTTON_ID_0] = BUTTON_ID_0;
	input->joystick.joystick_map[BUTTON_ID_1] = BUTTON_ID_1;
	
	(void)memset(input->mouse.mouse_map, 0xFF, sizeof(input->mouse.mouse_map));
	input->mouse.mouse_map[BUTTON_ID_0] = GLUT_LEFT_BUTTON;
	input->mouse.mouse_map[BUTTON_ID_1] = GLUT_RIGHT_BUTTON;
}

/*
 InputHardwareUpdate�֐�
 ���̓n�[�h�E�F�A�̏�Ԃ��X�V����
*/
void InputHardwareUpdate(void)
{
	// glutForceJoystickFunc();
}

#ifdef __cplusplus
}
#endif
