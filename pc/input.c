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

	if(x > STICK_THRESHOLD)
	{
		input->joystick.joystick[BUTTON_FLAG_RIGHT / (sizeof(unsigned int)*8)]
			|= BUTTON_FLAG_RIGHT;
	}
	else
	{		
		input->joystick.joystick[BUTTON_FLAG_RIGHT / (sizeof(unsigned int)*8)]
			&= ~(BUTTON_FLAG_RIGHT);
	}

	if(x < -STICK_THRESHOLD)
	{
		input->joystick.joystick[BUTTON_FLAG_LEFT / (sizeof(unsigned int)*8)]
			|= BUTTON_FLAG_LEFT;
	}
	else
	{		
		input->joystick.joystick[BUTTON_FLAG_LEFT / (sizeof(unsigned int)*8)]
			&= ~(BUTTON_FLAG_LEFT);
	}

	if(y > STICK_THRESHOLD)
	{
		input->joystick.joystick[BUTTON_FLAG_UP / (sizeof(unsigned int)*8)]
			|= BUTTON_FLAG_UP;
	}
	else
	{		
		input->joystick.joystick[BUTTON_FLAG_UP / (sizeof(unsigned int)*8)]
			&= ~(BUTTON_FLAG_UP);
	}

	if(y < - STICK_THRESHOLD)
	{
		input->joystick.joystick[BUTTON_FLAG_UP / (sizeof(unsigned int)*8)]
			|= BUTTON_FLAG_DOWN;
	}
	else
	{		
		input->joystick.joystick[BUTTON_FLAG_UP / (sizeof(unsigned int)*8)]
			&= ~(BUTTON_FLAG_DOWN);
	}

	input->joystick.joystick[0] = button >> (6);
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

#ifdef __cplusplus
}
#endif
