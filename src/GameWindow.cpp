#include "GameWindow.hpp"
#include "framework.hpp"
#include "ConfigManager.hpp"

namespace NovaEngine
{
	bool GameWindow::isGlfwInitialized_ = false;

	GameWindow::GameWindow(Engine* engine): engine_(engine), window_(nullptr)
	{
		if(!isGlfwInitialized_)
		{
			isGlfwInitialized_ = glfwInit() != GLFW_FALSE;
			if(!isGlfwInitialized_)
				throw "Could not initialize GLFW!\n";
		}
	}

	GameWindow::~GameWindow()
	{
		if(isGlfwInitialized_)
		{
			printf("terminating GLFW!...\n");
			glfwTerminate();
			isGlfwInitialized_ = false;
		}
	}

	bool GameWindow::create(const char* title, const GameWindowConfig& config)
	{
		if (window_ == nullptr)
		{
			glfwWindowHint(GLFW_RESIZABLE, config.resizable);
			glfwWindowHint(GLFW_MAXIMIZED, config.maximized);
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_VISIBLE, !config.hidden);

			window_ = glfwCreateWindow(config.width, config.height, title, nullptr, nullptr);

			if (window_ == nullptr)
			{
				printf("Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
				glfwTerminate();
				return false;
			}

			glfwSetWindowSizeLimits(window_, config.minWidth, config.minHeight, config.maxHeight, config.maxHeight);
			glfwSwapInterval(1);

			return true;
		}
		else
		{
			printf("Window already exists!");
			return false;
		}
	}

	bool GameWindow::isOpen()
	{
		return !glfwWindowShouldClose(window_);
	}

	void GameWindow::show()
	{
		if (window_ != nullptr)
			glfwShowWindow(window_);
	}

	void GameWindow::close()
	{
		if(window_ != nullptr)
		{
			printf("window::close()...\n");
			glfwDestroyWindow(window_);
		}
	}

	GLFWwindow* GameWindow::glfwWindow()
	{
		return window_;
	}
};
