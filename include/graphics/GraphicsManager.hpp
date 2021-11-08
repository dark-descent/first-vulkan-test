#ifndef ENGINE_GRAPHICS_GRAPHICS_MANAGER_HPP
#define ENGINE_GRAPHICS_GRAPHICS_MANAGER_HPP

#include "SubSystem.hpp"
#include "framework.hpp"
#include "graphics/VkFactory.hpp"

namespace NovaEngine::Graphics
{
	class GraphicsManager : public SubSystem<GLFWwindow*>
	{
	private:
		Vk::Instance instance_;
		Vk::Surface surface_;
		Vk::PhysicalDevice physicalDevice_;
		Vk::Device device_;
		Vk::SwapChain swapChain_;
		Vk::RenderPass renderPass_;
		Vk::Pipeline pipeline_;
		Vk::CommandPool commandPool_;
		Vk::CommandBufferGroup commandBuffers_;
		Vk::SyncObjects syncObjects_;

		ENGINE_SUB_SYSTEM_CTOR(GraphicsManager),
			instance_(),
			surface_(),
			physicalDevice_(),
			device_(),
			swapChain_(),
			renderPass_(),
			pipeline_(),
			commandPool_(),
			commandBuffers_(),
			syncObjects_()
		{};

	protected:
		bool onInitialize(GLFWwindow* window);
		bool onTerminate();

	public:
		void draw();
		void recordCommands(size_t index);

		friend class NovaEngine::Engine;
	};
};

#endif
