#include "game_data.h"
#include "initialize.h"

int main(int argc, char** argv)
{
<<<<<<< HEAD
	if(InitializeGameData(argc, argv) == FALSE)
=======
	if(InitializeGameData() == FALSE)
>>>>>>> 595175bcef89dcfa5ca4a1ef8275fb7d26b0dc89
	{
		return 1;
	}

	glutMainLoop();

	return 0;
}