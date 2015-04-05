#include <string.h>
#include "game_data.h"

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

	// 入力ハードウェアの状態を更新する
	InputHardwareUpdate();

	input->current_input = 0;

	// キーボードの状態から更新を実施(0～9, a～z, スペース, 改行)
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

	// キーボードの状態から更新を実施(矢印キー, PageUp, PageDown, Home, End, Insert)
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

	// ジョイスティックの状態から更新を実施
	for(i=0; i<NUM_BUTTON; i++)
	{
		if(input->joystick.joystick_map[i] != 0xFFFFFFFF)
		{
			if(input->joystick.joystick_map[i] <= BUTTON_ID_RIGHT)
			{
				if((input->joystick.stick & (1 << input->joystick.joystick_map[i])) != 0)
				{
					input->current_input |= (1 << i);
				}
			}
			else
			{
				if(((input->joystick.button << BUTTON_NUMER_ID_START)
						& (1 << (input->joystick.joystick_map[i]%(sizeof(unsigned int)*8))))
				)
				{
					input->current_input |= (1 << i);
				}
			}
		}
	}

	// マウスの状態から更新を実施
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
