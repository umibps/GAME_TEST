#include "game_data.h"
#include "initialize.h"

void TestTask(TASK* task)
{
	GAME_DATA *game_data = task->tasks->game_data;
	static int speed = 0;
	static int mode = 0;
	int i;

	for(i=0; i<(int)game_data->display_data.draw_items.num_data; i++)
	{
		((DRAW_ITEM_BASE*)game_data->display_data.draw_items.buffer[i].data)
			->draw((DRAW_ITEM_BASE*)game_data->display_data.draw_items.buffer[i].data);
	}

	{
		DRAW_SQUARE_ITEM *item0 = (DRAW_SQUARE_ITEM*)game_data->display_data.draw_items.buffer[0].data;
		CLIP_DRAW_ITEM *item1 = (CLIP_DRAW_ITEM*)game_data->display_data.draw_items.buffer[1].data;
		CLIP_DRAW_ITEM *item2 = (CLIP_DRAW_ITEM*)game_data->display_data.draw_items.buffer[2].data;
		
		SetTextureWrap(item2->texture->id, GL_REPEAT);

		if(game_data->input.input_down & BUTTON_FLAG_0)
		{
			speed = 0;
			if(mode == 0)
			{
				if((item1->color & 0xFF) < 180)
				{
					item1->color = RGBA(0xFF, 0x33, 0x33, (item1->color & 0xFF) + 2);
					item2->color = RGBA(0xFF, 0xFF, 0x44, (item2->color & 0xFF) + 2);
				}

				AddSound(&game_data->sounds, "sounds/voice.flac", FileArchiveReadNew,
					FileArchiveRead, FileArchiveSeek, FileArchiveTell, DeleteFileArchiveRead,
						SOUND_PLAY_NO_FLAG, &game_data->file_archive);
				mode = 1;
			}
			else
			{
				mode = 0;
				item1->color = 0;
				item2->color = 0;
			}
		}

		if((item1->color & 0xFF) > 0 && (item1->color & 0xFF) < 180)
		{
			if((item1->color & 0xFF) % 6 == 0)
			{
				speed++;
			}
			item1->color = RGBA(0xFF, 0x33, 0x33, (item1->color & 0xFF) + 2);
			item2->color = RGBA(0xFF, 0xFF, 0x88, (item2->color & 0xFF) + 2);
		}

		item2->texture_position[0][1] += speed;
		item2->texture_position[1][1] += speed;

		PlayAllSound(&game_data->sounds);
	}
}

int main(int argc, char** argv)
{
	GAME_DATA *game_data;

	if(InitializeGameData(argc, argv, GetGameData()) == FALSE)
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

	glutMainLoop();

	return 0;
}