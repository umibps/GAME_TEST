#include "game_data.h"
#include "initialize.h"

void TestTask(TASK* task)
{
	GAME_DATA *game_data = task->tasks->game_data;
	int i;

	for(i=0; i<(int)game_data->display_data.draw_items.num_data; i++)
	{
		((DRAW_ITEM_BASE*)game_data->display_data.draw_items.buffer[i].data)
			->draw((DRAW_ITEM_BASE*)game_data->display_data.draw_items.buffer[i].data);
	}

	{
		DRAW_SQUARE_ITEM *item1 = (DRAW_SQUARE_ITEM*)game_data->display_data.draw_items.buffer[1].data;
		DRAW_SQUARE_ITEM *item2 = (DRAW_SQUARE_ITEM*)game_data->display_data.draw_items.buffer[2].data;
		static int zoom = 100;
		static int direction = 4;

		item1->zoom = item2->zoom = zoom * 0.01f;
		zoom += direction;
		if(zoom > 300 || zoom < 50)
		{
			direction = -direction;
		}
		item1->rotate += (float)M_PI * 3 / 180.0f;
		item2->rotate += (float)M_PI * 3 / 180.0f;

		if(game_data->input.current_input & BUTTON_FLAG_UP)
		{
			item1->y -= 5;
			item2->y -= 5;
		}
		if(game_data->input.current_input & BUTTON_FLAG_DOWN)
		{
			item1->y += 5;
			item2->y += 5;
		}
		if(game_data->input.current_input & BUTTON_FLAG_LEFT)
		{
			item1->x -= 5;
			item2->x -= 5;
		}
		if(game_data->input.current_input & BUTTON_FLAG_RIGHT)
		{
			item1->x += 5;
			item2->x += 5;
		}
		if(game_data->input.input_down & BUTTON_FLAG_0)
		{
			if(item1->color == 0xFFFFFFFF)
			{
				item1->color = 0x00000000;
				item2->color = 0xFFFFFFFF;
			}
			else
			{
				item1->color = 0xFFFFFFFF;
				item2->color = 0x00000000;
			}
		}
	}
}

void SoundTestTask(TASK* task)
{
	PlaySound((SOUND_PLAY_BASE*)task->data);
}

int main(int argc, char** argv)
{
	GAME_DATA *game_data;
	TASK *task;
	FILE *fp;

	if(InitializeGameData(argc, argv) == FALSE)
	{
		return 1;
	}

	game_data = GetGameData();

	(void)TaskNew(&game_data->tasks,
		TestTask,
		NULL,
		0x1000,
		0
	);

	fp = fopen("./test.wav", "rb");

	task = TaskNew(&game_data->tasks,
		SoundTestTask,
		NULL,
		0x2000,
		sizeof(WAVE_SOUND_PLAY)
	);
	InitializeWaveSoundPlay(
		(WAVE_SOUND_PLAY*)task->data,
		&game_data->sound_context,
		fp,
		fread,
		fseek,
		ftell,
		fclose,
		SOUND_PLAY_FLAG_LOOP_PLAY
	);

	glutMainLoop();

	return 0;
}