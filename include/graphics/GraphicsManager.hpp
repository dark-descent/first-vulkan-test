#ifndef ENGINE_GRAPHICS_GRAPHICS_MANAGER_HPP
#define ENGINE_GRAPHICS_GRAPHICS_MANAGER_HPP

#include "SubSystem.hpp"
#include "framework.hpp"
#include "graphics/VkFactory.hpp"
#include "graphics/Context.hpp"

namespace NovaEngine
{
	class GameWindow;

	namespace Graphics
	{
		struct GraphicsConfig
		{

		};

		class GraphicsManager : public SubSystem<GraphicsConfig*>
		{
		private:
			static std::vector<const char*> defaultLayers;
			static const std::vector<const char*> defaultExtensions;

			VkDebugUtilsMessengerEXT debugExt;
			VkInstance instance_;
			std::vector<Context*> contexts_;

			ENGINE_SUB_SYSTEM_CTOR(GraphicsManager),
				debugExt(VK_NULL_HANDLE),
				instance_(VK_NULL_HANDLE),
				contexts_()
			{};

		protected:
			bool onInitialize(GraphicsConfig* config);
			bool onTerminate();

		public:
			Context* createContext(GLFWwindow* window, NovaEngine::Graphics::ContextOptions* options = nullptr);
			void destroyContext(Context* ctx);

			Context* getContextFromWindow(GameWindow* window);
			friend class Context;
			friend class SwapChain;
		};
	};
};

#endif
