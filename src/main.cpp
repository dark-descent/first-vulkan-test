#include "Engine.hpp"

#include "framework.hpp"
#include "Logger.hpp"

extern "C" void saveContext(NovaEngine::JobSystem::Context * ctx);
extern "C" void restoreContext(NovaEngine::JobSystem::Context * ctx);

int main(int argc, const char** argv)
{
	using namespace NovaEngine;


	volatile int x = 0;

	NovaEngine::JobSystem::Context c;
	saveContext(&c);

	printf("hello, world!\n");

	if (x == 0)
	{
		x++;
		restoreContext(&c);
	}

	return 0;

	// Engine engine;

	// if(!engine.initialize("Game.js"))
	// 	return 1;

	// engine.run();

	// if(!engine.terminate())
	// {
	// 	Logger::get()->info("oops??");
	// 	return 2;
	// }

	return 0;
}
