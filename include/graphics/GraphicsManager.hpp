#ifndef ENGINE_GRAPHICS_GRAPHICS_MANAGER_HPP
#define ENGINE_GRAPHICS_GRAPHICS_MANAGER_HPP

#include "SubSystem.hpp"
#include "framework.hpp"
#include "graphics/Context.hpp"

namespace NovaEngine::Graphics
{
	class GraphicsManager : public SubSystem<GLFWwindow*>
	{
	private:
		static const char* defaultAppName;
		
		Context ctx_;

		ENGINE_SUB_SYSTEM_CTOR(GraphicsManager), ctx_() {};

	protected:
		bool onInitialize(GLFWwindow* window);
		bool onTerminate();

		friend class NovaEngine::Engine;
	};
};

#endif
