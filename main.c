#include <time.h>
#include "game_data.h"
#include "initialize.h"
#include "random.h"

static void AddDrawItems(GAME_DATA* game_data)
{
	int i;
	for(i=0; i<5; i++)
	{
		PriorityArrayAppend(&game_data->display_data.draw_items,
			DrawSquareItemNew(TextureCountUpReference(game_data->textures[game_data->order[i]]),
				1440 + i * 300, 300, 1, 0, RGBA(255, 255, 255, 255), &game_data->display_data.programs), i);
	}
}

void TestTask(TASK* task)
{
#define SPEED 30
	GAME_DATA *game_data = task->tasks->game_data;
	static int initialized = 0;
	int i;

	for(i=0; i<(int)game_data->display_data.draw_items.num_data; i++)
	{
		((DRAW_ITEM_BASE*)game_data->display_data.draw_items.buffer[i].data)
			->draw((DRAW_ITEM_BASE*)game_data->display_data.draw_items.buffer[i].data);
	}

	{
		DRAW_SQUARE_ITEM *item0;
		DRAW_SQUARE_ITEM *item1;
		DRAW_SQUARE_ITEM *item2;
		DRAW_SQUARE_ITEM *item3;
		DRAW_SQUARE_ITEM *item4;
		
		if(game_data->input.input_down & BUTTON_FLAG_0)
		{
			RANROT ranrot;
			SeedRanrot(&ranrot, (random_uint32)time(NULL));
			ShuffleArray(game_data->order, 53, -1, RanrotGetNext, &ranrot);
			initialized = 1;
			PriorityArrayReset(&game_data->display_data.draw_items);
			AddDrawItems(game_data);
		}
		else if(game_data->input.input_down & BUTTON_FLAG_1)
		{
			XOR_SHIFT32 xor_shift;
			SeedXorShift32(&xor_shift, (random_uint32)time(NULL));
			ShuffleArray(game_data->order, 53, -1, XorShift32GetNext, &xor_shift);
			initialized = 1;
			PriorityArrayReset(&game_data->display_data.draw_items);
			AddDrawItems(game_data);
		}
		else if(game_data->input.input_down & BUTTON_FLAG_2)
		{
			XOR_SHIFT128 xor_shift;
			SeedXorShift128(&xor_shift, (random_uint32)time(NULL));
			ShuffleArray(game_data->order, 53, -1, XorShift128GetNext, &xor_shift);
			initialized = 1;
			PriorityArrayReset(&game_data->display_data.draw_items);
			AddDrawItems(game_data);
		}

		
		item0 = (DRAW_SQUARE_ITEM*)game_data->display_data.draw_items.buffer[0].data;
		item1 = (DRAW_SQUARE_ITEM*)game_data->display_data.draw_items.buffer[1].data;
		item2 = (DRAW_SQUARE_ITEM*)game_data->display_data.draw_items.buffer[2].data;
		item3 = (DRAW_SQUARE_ITEM*)game_data->display_data.draw_items.buffer[3].data;
		item4 = (DRAW_SQUARE_ITEM*)game_data->display_data.draw_items.buffer[4].data;

		if(initialized != 0)
		{
			if((item0->x - 1440) * (item0->x - 1440) < SPEED * SPEED)
			{
				AddSound(&game_data->sounds, "sounds/draw.flac", FileArchiveReadNew,
					FileArchiveRead, FileArchiveSeek, FileArchiveTell, DeleteFileArchiveRead,
						SOUND_PLAY_NO_FLAG, &game_data->file_archive);
			}
			if((item1->x - 1440) * (item1->x - 1440) < SPEED * SPEED)
			{
				AddSound(&game_data->sounds, "sounds/draw.flac", FileArchiveReadNew,
					FileArchiveRead, FileArchiveSeek, FileArchiveTell, DeleteFileArchiveRead,
						SOUND_PLAY_NO_FLAG, &game_data->file_archive);
			}
			if((item2->x - 1440) * (item2->x - 1440) < SPEED * SPEED)
			{
				AddSound(&game_data->sounds, "sounds/draw.flac", FileArchiveReadNew,
					FileArchiveRead, FileArchiveSeek, FileArchiveTell, DeleteFileArchiveRead,
						SOUND_PLAY_NO_FLAG, &game_data->file_archive);
			}
			if((item3->x - 1440) * (item3->x - 1440) < SPEED * SPEED)
			{
				AddSound(&game_data->sounds, "sounds/draw.flac", FileArchiveReadNew,
					FileArchiveRead, FileArchiveSeek, FileArchiveTell, DeleteFileArchiveRead,
						SOUND_PLAY_NO_FLAG, &game_data->file_archive);
			}
			if((item4->x - 1440) * (item4->x - 1440) < SPEED * SPEED)
			{
				AddSound(&game_data->sounds, "sounds/draw.flac", FileArchiveReadNew,
					FileArchiveRead, FileArchiveSeek, FileArchiveTell, DeleteFileArchiveRead,
						SOUND_PLAY_NO_FLAG, &game_data->file_archive);
			}

			if(item0->x > 60)
			{
				item0->x -= SPEED;
			}
			else if(item0->x < 60)
			{
				item0->x = 60;
			}
			if(item1->x > 280)
			{
				item1->x -= SPEED;
			}
			else if(item1->x < 280)
			{
				item1->x = 280;
			}
			if(item2->x > 500)
			{
				item2->x -= SPEED;
			}
			else if(item2->x < 500)
			{
				item2->x = 500;
			}
			if(item3->x > 720)
			{
				item3->x -= SPEED;
			}
			else if(item3->x < 720)
			{
				item3->x = 720;
			}
			if(item4->x > 940)
			{
				item4->x -= SPEED;
			}
			else if(item4->x < 940)
			{
				item4->x = 940;
			}
		}

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