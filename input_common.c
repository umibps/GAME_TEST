#include <string.h>
#include "game_data.h"

/*
 UpdateInput�֐�
 ���͏�Ԃ��X�V����
 ����
 input	: ���͏����S�̂��Ǘ�����f�[�^
*/
void UpdateInput(GAME_INPUT* input)
{
	uint32 before_input = input->current_input;
	int i;

	// ���̓n�[�h�E�F�A�̏�Ԃ��X�V����
	InputHardwareUpdate();

	input->current_input = 0;

	// �L�[�{�[�h�̏�Ԃ���X�V�����{(0�`9, a�`z, �X�y�[�X, ���s)
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

	// �L�[�{�[�h�̏�Ԃ���X�V�����{(���L�[, PageUp, PageDown, Home, End, Insert)
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

	// �W���C�X�e�B�b�N�̏�Ԃ���X�V�����{
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

	// �}�E�X�̏�Ԃ���X�V�����{
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

	// �{�^���������ꂽ�t���[���̂�ON�ɂȂ�o�b�t�@��
		// �{�^���������ꂽ�t���[���̂�ON�ɂȂ�o�b�t�@���X�V
	input->input_down = input->current_input & (~ (before_input));
	input->intpu_up = (~ (input->current_input)) & before_input;
}
