#include "../game_data.h"
#include "../configure.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 KeyboardDownCallback関数
 キーボードのキーが押された時のコールバック関数
 引数
 key	: 押されたキー
 x		: キーが押された時のマウスのX座標
 y		: キーが押された時のマウスのY座標
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
 KeyboardUpCallback関数
 キーボードのキーが離された時のコールバック関数
 引数
 key	: 離されたキー
 x		: キーが離された時のマウスのX座標
 y		: キーが離された時のマウスのY座標
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
 KeyboardSpecialDownCallback関数
 キーボードの特殊キーが押された時のコールバック関数
 引数
 key	: 押された特殊キー
 x		: 特殊キーが押された時のマウスのX座標
 y		: 特殊キーが押された時のマウスのY座標
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
 KeyboardSpecialUpCallback関数
 キーボードの特殊キーが離された時のコールバック関数
 引数
 key	: 離された特殊キー
 x		: 特殊キーが離された時のマウスのX座標
 y		: 特殊キーが離された時のマウスのY座標
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
 JoystickCallback関数
 ジョイスティックの状態が更新された時のコールバック関数
 引数
 button	: 状態が更新されたボタン
 x		: スティックのX座標
 y		: スティックのY座標
 z		: スティックのZ座標
*/
void JoystickCallback(unsigned int button, int x, int y, int z)
{
#define STICK_THRESHOLD (300)	// スティックの判定閾値
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
 MouseCallback関数
 マウスの状態が更新された時のコールバック関数
 引数
 button	: 状態の変更があったボタン
 state	: ボタンの状態
 x		: マウスのX座標
 y		: マウスのY座標	
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
 SetInputCallbacks関数
 入力に対するコールバック関数を設定する
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
 LoadDefaultInputSettings関数
 入力設定のデフォルトデータを設定する
 引数
 input	: 入力処理全体を管理するデータ
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
 InputHardwareUpdate関数
 入力ハードウェアの状態を更新する
*/
void InputHardwareUpdate(void)
{
	// glutForceJoystickFunc();
}

#ifdef __cplusplus
}
#endif
