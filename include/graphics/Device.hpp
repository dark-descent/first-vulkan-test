#ifndef ENGINE_GRAPHICS_DEVICE_HPP
#define ENGINE_GRAPHICS_DEVICE_HPP

#include "framework.hpp"
#include "graphics/GfxObject.hpp"
#include "graphics/PhysicalDevice.hpp"

namespace NovaEngine::Graphics
{
	typedef void(*RecordCallback)();

	struct DeviceConfig
	{
		uint32_t queueCount;
	};

	class Device : public GfxObject<DeviceConfig*>
	{
	private:
		static DeviceConfig defaultConfig;

		VkDevice device_;
		std::vector<VkQueue> graphicsQueues_;
		VkQueue presentQueue_;
		VkCommandPool commandPool_;
		std::vector<VkCommandBuffer> commandBuffers_;

		GFX_CTOR(Device),
			device_(VK_NULL_HANDLE),
			graphicsQueues_(),
			presentQueue_(VK_NULL_HANDLE),
			commandPool_(VK_NULL_HANDLE),
			commandBuffers_()
		{}

	protected:
		bool onInitialize(DeviceConfig* config);
		bool onTerminate();

	public:
		const std::vector<VkQueue>& graphicsQueues();
		VkQueue getGraphicsQueue(uint32_t index);
		VkCommandPool& commandPool();
		VkQueue& presentQueue();

		bool initCommandBuffers();
		VkCommandBuffer getCommandBuffer(size_t index);

		VkDevice operator*() { return device_; }

		bool record(size_t frameIndex, RecordCallback callback);
	};

}


#endif
