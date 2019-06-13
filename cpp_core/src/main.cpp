#include <CMake_Learning/game_interface.h>

int main()
{
	GameInterface g(5);
	g.play(0);
	g.play(1);
	g.play(2);
	g.play(4);
	g.play(4);
	g.play(6);
	return 0;
}