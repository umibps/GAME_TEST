#include "game_data.h"
#include "initialize.h"

int main(int argc, char** argv)
{
	if(InitializeGameData(argc, argv) == FALSE)
	{
		return 1;
	}

	glutMainLoop();

	return 0;
}