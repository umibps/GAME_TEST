#include <string.h>
#include "game_data.h"

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
	input->joystick.joystick_map[BUTTON_ID_UP] = BUTTON_FLAG_UP;
	input->joystick.joystick_map[BUTTON_ID_DOWN] = BUTTON_FLAG_DOWN;
	input->joystick.joystick_map[BUTTON_ID_LEFT] = BUTTON_FLAG_LEFT;
	input->joystick.joystick_map[BUTTON_ID_RIGHT] = BUTTON_FLAG_RIGHT;
	input->joystick.joystick_map[BUTTON_ID_0] = BUTTON_FLAG_0;
	input->joystick.joystick_map[BUTTON_ID_1] = BUTTON_FLAG_1;
	
	(void)memset(input->mouse.mouse_map, 0xFF, sizeof(input->mouse.mouse_map));
	input->mouse.mouse_map[BUTTON_ID_0] = GLUT_LEFT_BUTTON;
	input->mouse.mouse_map[BUTTON_ID_1] = GLUT_RIGHT_BUTTON;
}

/*
 UpdateInput関数
 入力状態を更新する
 引数
 input	: 入力処理全体を管理するデータ
*/
void UpdateInput(GAME_INPUT* input)
{
	uint32 before_input = input->current_input;
	int i;

	input->current_input = 0;
	for(i=0; i<NUM_BUTTON; i++)
	{
		if(input->keyboard.keyboard_map[i] != 0xFFFFFFFF)
		{
			if((input->keyboard.keyboard[
				input->keyboard.keyboard_map[i]/(sizeof(unsigned int)*8)]
					& (1 << (input->keyboard.keyboard_map[i]%(sizeof(unsigned int)*8))))
			)
			{
				input->current_input |= (1 << i);
			}
		}
	}

	for(i=0; i<NUM_BUTTON; i++)
	{
		if(input->keyboard.special_map[i] != 0xFFFFFFFF)
		{
			if((input->keyboard.special[
				input->keyboard.special_map[i]/(sizeof(unsigned int)*8)]
					& (1 << (input->keyboard.special_map[i]%(sizeof(unsigned int)*8))))
			)
			{
				input->current_input |= (1 << i);
			}
		}
	}

	for(i=0; i<NUM_BUTTON; i++)
	{
		if(input->joystick.joystick_map[i] != 0xFFFFFFFF)
		{
			if((input->joystick.joystick[
				input->joystick.joystick_map[i]/(sizeof(unsigned int)*8)]
					& (1 << (input->joystick.joystick_map[i]%(sizeof(unsigned int)*8))))
			)
			{
				input->current_input |= (1 << i);
			}
		}
	}

	for(i=0; i<NUM_BUTTON; i++)
	{
		if(input->mouse.mouse_map[i] != 0xFFFFFFFF)
		{
			if((input->mouse.mouse[
				input->mouse.mouse_map[i]/(sizeof(unsigned int)*8)]
					& (1 << (input->mouse.mouse_map[i]%(sizeof(unsigned int)*8))))
			)
			{
				input->current_input |= (1 << i);
			}
		}
	}

	// ボタンが押されたフレームのみONになるバッファと
		// ボタンが離されたフレームのみONになるバッファを更新
	input->input_down = input->current_input & (~ (before_input));
	input->intpu_up = (~ (input->current_input)) & before_input;
}
