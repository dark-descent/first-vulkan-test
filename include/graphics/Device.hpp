#ifndef ENGINE_GRAPHICS_DEVICE_HPP
#define ENGINE_GRAPHICS_DEVICE_HPP

#include "framework.hpp"
#include "graphics/GfxObject.hpp"
#include "graphics/PhysicalDevice.hpp"

namespace NovaEngine::Graphics
{
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

		GFX_CTOR(Device), device_(VK_NULL_HANDLE) {}

	protected:
		bool onInitialize(DeviceConfig* config);
		bool onTerminate();

	public:
		const std::vector<VkQueue>& graphicsQueues();

		VkQueue getGraphicsQueue(uint32_t index);
		
		VkDevice operator*() { return device_; }

	};

}


#endif
