#include "GameWindow.hpp"
#include "framework.hpp"
#include "ConfigManager.hpp"
#include "Logger.hpp"

namespace NovaEngine
{
	GameWindow* GameWindow::firstWindow_ = nullptr;
	bool GameWindow::isGlfwInitialized_ = false;

	GameWindow::GameWindow(Engine* engine) : engine_(engine), window_(nullptr)
	{
		if (!isGlfwInitialized_)
		{
			isGlfwInitialized_ = glfwInit() != GLFW_FALSE;
			if (!isGlfwInitialized_)
				throw "Could not initialize GLFW!";
		}
	}

	GameWindow::~GameWindow()
	{
		
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
				Logger::get()->error("Failed to open GLFW window.");
				return false;
			}

			if(firstWindow_ == nullptr)
				firstWindow_ = this;

			glfwSetWindowSizeLimits(window_, config.minWidth, config.minHeight, config.maxHeight, config.maxHeight);
			glfwSwapInterval(1);

			return true;
		}
		else
		{
			Logger::get()->warn("Window already exists!");
			return false;
		}
	}

	bool GameWindow::isClosed()
	{
		return glfwWindowShouldClose(window_);
	}

	void GameWindow::show()
	{
		if (window_ != nullptr)
			glfwShowWindow(window_);
	}

	void GameWindow::destroy()
	{
		if (window_ != nullptr)
		{
			Logger::get()->info("window::close()...");
			glfwDestroyWindow(window_);
			window_ = nullptr;
		}
	}

	GLFWwindow* GameWindow::glfwWindow()
	{
		return window_;
	}
};
