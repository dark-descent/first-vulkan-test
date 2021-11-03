#include "GraphicsManager.hpp"
#include "Engine.hpp"
#include "ConfigManager.hpp"
#include "graphics/Context.hpp"

namespace NovaEngine
{

	namespace
	{
		Graphics::Context ctx_;
	};

	bool GraphicsManager::onInitialize(GLFWwindow* window)
	{
		EngineConfig* config = engine()->configManager.getConfig();

		CHECK_OR_RETURN(window, "No window provided!");

		const std::vector<const char*> validationLayers = {
				"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		
		if(!ctx_.isInitialized())
		{
			printf("initializing context...\n");
			CHECK_OR_RETURN(ctx_.initialize(config->name.c_str(), window, validationLayers, deviceExtensions), "Could not create vulkan context!");
		}
		else
		{
			printf("Context was somehow already initialized???\n");
			return false;
		}
		
		return true;
	}

	bool GraphicsManager::onTerminate()
	{
		ctx_.terminate();
		return true;
	}
};
