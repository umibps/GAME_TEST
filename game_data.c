#include "game_data.h"

GAME_DATA* GetGameData(void)
{
	static GAME_DATA game_data ={0};

	return &game_data;
}
