#include "game_data.h"
#include "initialize.h"

void TestTask(TASK* task)
{
	GAME_DATA *game_data = task->tasks->game_data;
	int i;

	for(i=0; i<game_data->display_data.draw_items.num_data; i++)
	{
		((DRAW_ITEM_BASE*)game_data->display_data.draw_items.buffer[i].data)
			->draw((DRAW_ITEM_BASE*)game_data->display_data.draw_items.buffer[i].data);
	}

	{
		DRAW_SQUARE_ITEM *item = (DRAW_SQUARE_ITEM*)game_data->display_data.draw_items.buffer[1].data;
		static int zoom = 100;
		static int direction = 4;

		item->zoom = zoom * 0.01f;
		zoom += direction;
		if(zoom > 300 || zoom < 50)
		{
			direction = -direction;
		}
		item->rotate += (float)M_PI * 3 / 180.0f;

		if(game_data->input.current_input & BUTTON_FLAG_UP)
		{
			item->y -= 5;
		}
		if(game_data->input.current_input & BUTTON_FLAG_DOWN)
		{
			item->y += 5;
		}
		if(game_data->input.current_input & BUTTON_FLAG_LEFT)
		{
			item->x -= 5;
		}
		if(game_data->input.current_input & BUTTON_FLAG_RIGHT)
		{
			item->x += 5;
		}
	}
}

int main(int argc, char** argv)
{
	GAME_DATA *game_data;
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

	glutMainLoop();

	return 0;
}