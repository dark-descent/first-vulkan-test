#include "Engine.hpp"

#include "framework.hpp"

int main(int argc, const char** argv)
{
	using namespace NovaEngine;
	
	Engine engine;

	if(!engine.initialize("Game.js"))
		return 1;
	
	engine.run();

	if(!engine.terminate())
	{
		printf("oops??\n");
		return 2;
	}
	
	return 0;
}
