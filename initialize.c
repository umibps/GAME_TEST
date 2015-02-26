#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "game_data.h"
#include "display.h"

int InitializeGameData(int argc, char** argv)
{
	GAME_DATA *game_data = GetGameData();

	(void)memset(game_data, 0, sizeof(*game_data));

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	
	glutInitWindowSize(640, 360);

	(void)glutCreateWindow("GAME TEST");


	glutDisplayFunc((void(*)(void))Display);

	if(glewInit() != GLEW_OK)
	{
		return FALSE;
	}

	glClearColor(0, 0, 0, 0);

	{
		InitializeImageTexture(&game_data->texture, "test.png",
			fopen, fread, fseek, ftell, fclose, game_data);
		InitializeSimpleProgram(&game_data->programs.draw_square);
		InitializeVertexBuffer(
			&game_data->vertex_buffer, sizeof(VERTEX_BUFFER)*4, sizeof(int)*4,
			sizeof(DRAW_VERTEX), SHADER_ATTRIBUTE_VERTEX, (void*)0,
			SHADER_ATTRIBUTE_TEXTURE_COORD, (void*)offsetof(DRAW_VERTEX, texture_coord),
			SHADER_ATTRIBUTE_COLOR, (void*)offsetof(DRAW_VERTEX, color));
	}

	return TRUE;
}
