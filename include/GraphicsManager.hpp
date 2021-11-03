#ifndef ENGINE_RENDER_MANAGER_HPP
#define ENGINE_RENDER_MANAGER_HPP

#include "framework.hpp"
#include "SubSystem.hpp"
#include "graphics/Context.hpp"

namespace NovaEngine
{
	class GraphicsManager : public SubSystem<GLFWwindow*>
	{
	private:
		Graphics::Context ctx_;
		ENGINE_SUB_SYSTEM_CTOR(GraphicsManager), ctx_() {};

	protected:
		bool onInitialize(GLFWwindow* window);
		bool onTerminate();
	};
}

#endif
