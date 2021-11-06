#include "graphics/GraphicsManager.hpp"

#include "Engine.hpp"

namespace NovaEngine::Graphics
{
	bool GraphicsManager::onInitialize(GLFWwindow* window)
	{
		ctx_.initialize(engine()->configManager.getConfig()->name.c_str(), window);

		return true;
	}

	bool GraphicsManager::onTerminate()
	{
		ctx_.terminate();
		return true;
	}
};
