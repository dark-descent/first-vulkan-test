#ifndef ENGINE_GAME_WINDOW_HPP
#define ENGINE_GAME_WINDOW_HPP

#include "framework.hpp"

#include "EngineConfig.hpp"

namespace NovaEngine
{
	class Engine;

	class GameWindow
	{
	public:

		GameWindow(Engine* engine);

		bool create(const char* title, const GameWindowConfig& config);

		/** @returns true if the game window went into fullscreen mode */
		bool toggleFullScreen();

		bool isOpen();

		void show();

		void close();

		GLFWwindow* glfwWindow();

	private:
		Engine* engine_;
		GLFWwindow* window_;
	};
}
#endif
