#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <thread>

#include <KinectIP/game_interface.h>
#include <NDI/Processing.NDI.Lib.h>

int main(int argc, char* argv[])
{	// Not required, but "correct" (see the SDK documentation)
	std::cout << "hello world!" << std::endl;
	if (!NDIlib_initialize())
	{	// Cannot run NDI. Most likely because the CPU is not sufficient (see SDK documentation).
		// you can check this directly with a call to NDIlib_is_supported_CPU()
		printf("Cannot run NDI.");
		return 0;
	}
}