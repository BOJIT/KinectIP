#include <game_engine.h>
#include <KinectIP/game_interface.h>

void GameInterface::play(int num)
{
	this->game->guess(num);
}

GameInterface::GameInterface(int length)
{
	this->game = new GameEngine(length);
}