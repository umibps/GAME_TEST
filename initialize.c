#ifdef _MSC_VER
// 埋め込み文字列をUTF-8にする
# pragma execution_character_set("utf-8")
#endif

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "game_data.h"
#include "display.h"
#include "main_loop.h"
#include "utils.h"
#include "task_common.h"
#include "configure.h"

int InitializeGameData(int argc, char** argv)
{
	DRAW_VERTEX vertices[4] =
	{
		{{-1, 1, 0}, {0, 0}, {255, 255, 255, 255}},
		{{-1, -1, 0}, {0, 1}, {255, 255, 255, 255}},
		{{1, -1, 0}, {1, 1}, {255, 255, 255, 255}},
		{{1, 1, 0}, {1, 0}, {255, 255, 255, 255}}
	};
	unsigned char indices[] ={0, 1, 2, 3};

	GAME_DATA *game_data = GetGameData();

	(void)memset(game_data, 0, sizeof(*game_data));

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	
	glutInitWindowSize(640, 360);

	(void)glutCreateWindow("GAME TEST");

	InitializeTextDrawFromFile(&game_data->display_data.text_draw, "./font_1_honokamin.ttf");
	TextDrawSetCharacterSize(&game_data->display_data.text_draw, 80, 80);

	glutDisplayFunc((void(*)(void))Display);
	glutReshapeFunc((void(*)(int, int))ReshapeWindow);
	glutTimerFunc(1000 / FRAMES_PER_SECOND,
		(void (*)(int))TimerCallback, 0);

	SetInputCallbacks();
	LoadDefaultInputSettings(&game_data->input);

	if(glewInit() != GLEW_OK)
	{
		return FALSE;
	}

	glClearColor(0, 0, 0, 0);

	InitializeDisplayData(&game_data->display_data);

	InitializeTasks(&game_data->tasks, game_data);
	(void)TaskNew(&game_data->tasks,
		(void (*)(TASK*))TaskTop,
		NULL,
		0x00000000,
		0
	);
	(void)TaskNew(&game_data->tasks,
		(void (*)(TASK*))TaskBottom,
		NULL,
		0xFFFFFFFF,
		0
	);

	{
		/*
		PriorityArrayAppend(&game_data->display_data.draw_items,
			DrawSquareItemNew(
			ImageTextureNew("test.jpg",
				FileOpen, fread, fseek, ftell, FileClose, game_data, &game_data->display_data.textures),
			0, 0,
			0.33f,
			0,
			RGBA(0xFF, 0xFF, 0xFF, 0xFF),
			&game_data->display_data.programs),
			0
		);
		*/

		PriorityArrayAppend(&game_data->display_data.draw_items,
			DrawSquareItemNew(
			ImageTextureNew("test.png",
				FileOpen, fread, fseek, ftell, FileClose, game_data, &game_data->display_data.textures),
			300, 260,
			2,
			0,
			RGBA(0xFF, 0xFF, 0xFF, 0xFF),
			&game_data->display_data.programs),
			0
		);

		PriorityArrayAppend(&game_data->display_data.draw_items,
			DrawSquareItemNew(
			ImageTextureNew("flame.png",
				FileOpen, fread, fseek, ftell, FileClose, game_data, &game_data->display_data.textures),
			500, 260,
			1,
			0,
			RGBA(0xFF, 0xFF, 0xFF, 0xFF),
			&game_data->display_data.programs),
			0
		);

		PriorityArrayAppend(&game_data->display_data.draw_items,
			DrawSquareItemNew(
			ImageTextureNew("flame_blue.png",
				FileOpen, fread, fseek, ftell, FileClose, game_data, &game_data->display_data.textures),
			500, 260,
			1,
			0,
			RGBA(0x00, 0x00, 0x00, 0x00),
			&game_data->display_data.programs),
			0
		);
	}

	return TRUE;
}
