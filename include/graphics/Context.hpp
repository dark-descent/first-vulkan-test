#ifndef ENGINE_GRAPHICS_CONTEXT_HPP
#define ENGINE_GRAPHICS_CONTEXT_HPP

#include "framework.hpp"
#include "SwapChain.hpp"
#include "Initializable.hpp"

namespace NovaEngine::Graphics
{
	class GraphicsManager;

	class Context : public Initializable<GraphicsManager*, GLFWwindow*, VkSurfaceKHR, SwapChainOptions*>
	{
		GraphicsManager* manager_;
		GLFWwindow* window_;
		VkSurfaceKHR surface_;
		SwapChain swapChain_;
		VkRenderPass renderPass_;

	protected:
		bool onInitialize(GraphicsManager* manager, GLFWwindow* window, VkSurfaceKHR surface = VK_NULL_HANDLE, SwapChainOptions* options = nullptr);

	public:
		Context() : Initializable(),
			manager_(nullptr),
			window_(nullptr),
			surface_(VK_NULL_HANDLE),
			swapChain_(),
			renderPass_(VK_NULL_HANDLE)
		{}


		GLFWwindow* window() { return window_; }
		const VkSurfaceKHR& surface() { return surface_; }
		SwapChain& swapChain() { return swapChain_; }

		void destroy();

		friend class GraphicsManager;
		friend class SwapChain;
	};
};

#endif
