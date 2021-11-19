#ifndef ENGINE_GAME_WINDOW_HPP
#define ENGINE_GAME_WINDOW_HPP

#include "framework.hpp"

#include "EngineConfig.hpp"

namespace NovaEngine
{
	class Engine;

	class GameWindow
	{
		static bool isGlfwInitialized_;
		static GameWindow* firstWindow_;

		GameWindow(Engine* engine);
	
	public:
		~GameWindow();
	
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

		friend class Engine;
	};
}
#endif
