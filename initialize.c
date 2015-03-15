#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "game_data.h"
#include "display.h"

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


	glutDisplayFunc((void(*)(void))Display);

	if(glewInit() != GLEW_OK)
	{
		return FALSE;
	}

	glClearColor(0, 0, 0, 0);

	{
		InitializeImageTexture(&game_data->texture1, "test.jpg",
			fopen, fread, fseek, ftell, fclose, game_data);
		InitializeImageTexture(&game_data->texture2, "test.png",
			fopen, fread, fseek, ftell, fclose, game_data);
		InitializeSimpleProgram(&game_data->programs.draw_square);
		InitializeVertexBuffer(
			&game_data->vertex_buffer, sizeof(vertices), vertices,
			sizeof(indices), indices, sizeof(DRAW_VERTEX),
			3, SHADER_ATTRIBUTE_VERTEX, GL_FLOAT, GL_FALSE, 0,
			2, SHADER_ATTRIBUTE_TEXTURE_COORD, GL_FLOAT, GL_FALSE, offsetof(DRAW_VERTEX, texture_coord),
			4, SHADER_ATTRIBUTE_COLOR, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(DRAW_VERTEX, color),
			0
		);
	}

	return TRUE;
}
