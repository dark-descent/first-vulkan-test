#include "graphics/GraphicsManager.hpp"

#include "Engine.hpp"

namespace NovaEngine::Graphics
{
	std::vector<const char*> GraphicsManager::defaultLayers = {
#ifdef DEBUG
			"VK_LAYER_KHRONOS_validation",
#endif
	};

	const std::vector<const char*> GraphicsManager::defaultExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	bool GraphicsManager::onInitialize(GraphicsConfig* config)
	{
		contexts_.reserve(10);
		for (size_t i = 0; i < 10; i++)
			contexts_[i] = nullptr;
		instance_ = VkFactory::createInstance(defaultLayers, &debugExt);

		return true;
	}

	bool GraphicsManager::onTerminate()
	{
		for (size_t i = 0; i < contexts_.size(); i++)
		{
			if (contexts_[i] != nullptr)
			{
				contexts_[i]->destroy();
				contexts_[i] = nullptr;
			}
		}
		return true;
	}

	Context* GraphicsManager::createContext(GLFWwindow* window, NovaEngine::Graphics::ContextOptions* options)
	{
		for (size_t i = 0; i < 10; i++)
			if (contexts_[i] == nullptr)
			{
				contexts_.push_back(new Context(i));
				contexts_[i]->initialize(this, window, options);
				return contexts_[i];
			}
		return nullptr;
	}

	void GraphicsManager::destroyContext(Context* ctx)
	{
		for (size_t i = 0; i < 10; i++)
			if (contexts_[i] == ctx)
			{
				contexts_[i]->destroy();
				contexts_[i] = nullptr;
			}
	}

	Context* GraphicsManager::getContextFromWindow(GameWindow* window)
	{
		GLFWwindow* w = window->glfwWindow();
		for (const auto& ctx : contexts_)
			if (ctx != nullptr && ctx->window_ == w)
				return ctx;
		return nullptr;
	}
};
