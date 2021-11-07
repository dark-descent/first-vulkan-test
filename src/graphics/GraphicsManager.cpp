#include "graphics/GraphicsManager.hpp"

#include "Engine.hpp"

namespace NovaEngine::Graphics
{
	bool GraphicsManager::onInitialize(GLFWwindow* window)
	{
		if(!ctx_.initialize(engine(), engine()->configManager.getConfig()->name.c_str(), window))
			return false;

		return true;
	}

	bool GraphicsManager::onTerminate()
	{
		ctx_.terminate();
		return true;
	}
};
