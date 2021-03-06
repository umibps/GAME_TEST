﻿#ifdef _MSC_VER
// 埋め込み文字列をUTF-8にする
# pragma execution_character_set("utf-8")
# if defined(_M_IX86)
#  pragma comment(lib, "./lib_x86/OpenAL32.lib")
# else
#  pragma comment(lib, "./lib_x64/OpenAL32.lib")
# endif
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

int InitializeGameData(int argc, char** argv, GAME_DATA* game_data)
{
	DRAW_VERTEX vertices[4] =
	{
		{{-1, 1, 0}, {0, 0}, {255, 255, 255, 255}},
		{{-1, -1, 0}, {0, 1}, {255, 255, 255, 255}},
		{{1, -1, 0}, {1, 1}, {255, 255, 255, 255}},
		{{1, 1, 0}, {1, 0}, {255, 255, 255, 255}}
	};
	unsigned char indices[] ={0, 1, 2, 3};
	FILE *fp;

	(void)memset(game_data, 0, sizeof(*game_data));

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	
	glutInitWindowSize(640, 360);

	(void)glutCreateWindow("GAME TEST");

	fp = fopen("./test.bin", "rb");
	if(InitializeFileArchive(&game_data->file_archive, "./test.bin", (void*)fp,
		(size_t(*)(void*, size_t, size_t, void*))fread,
		(int(*)(void*, long, int))fseek,
		(long(*)(void*))ftell,
		(int(*)(void*))fclose) == FALSE)
	{
		return FALSE;
	}

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

	InitializeSoundContext(&game_data->sound_context);

	InitializeSounds(&game_data->sounds, &game_data->sound_context);

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
		char file_name[256];
		int num_texture = 0;
		int i;

		for(i=1; i<=13; i++)
		{
			(void)sprintf(file_name, "images/c%02d.png", i);
			game_data->textures[num_texture++] = ImageTextureNew(file_name,
				FileArchiveReadNew, FileArchiveRead, FileArchiveSeek, FileArchiveTell, DeleteFileArchiveRead,
					&game_data->file_archive, &game_data->display_data.textures);
		}
		for(i=1; i<=13; i++)
		{
			(void)sprintf(file_name, "images/d%02d.png", i);
			game_data->textures[num_texture++] = ImageTextureNew(file_name,
				FileArchiveReadNew, FileArchiveRead, FileArchiveSeek, FileArchiveTell, DeleteFileArchiveRead,
					&game_data->file_archive, &game_data->display_data.textures);
		}
		for(i=1; i<=13; i++)
		{
			(void)sprintf(file_name, "images/h%02d.png", i);
			game_data->textures[num_texture++] = ImageTextureNew(file_name,
				FileArchiveReadNew, FileArchiveRead, FileArchiveSeek, FileArchiveTell, DeleteFileArchiveRead,
					&game_data->file_archive, &game_data->display_data.textures);
		}
		for(i=1; i<=13; i++)
		{
			(void)sprintf(file_name, "images/s%02d.png", i);
			game_data->textures[num_texture++] = ImageTextureNew(file_name,
				FileArchiveReadNew, FileArchiveRead, FileArchiveSeek, FileArchiveTell, DeleteFileArchiveRead,
					&game_data->file_archive, &game_data->display_data.textures);
		}
		game_data->textures[num_texture++] = ImageTextureNew("images/x01.png",
			FileArchiveReadNew, FileArchiveRead, FileArchiveSeek, FileArchiveTell, DeleteFileArchiveRead,
					&game_data->file_archive, &game_data->display_data.textures);
	}

	return TRUE;
}
