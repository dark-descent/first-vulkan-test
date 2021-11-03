#ifndef ENGINE_ENGINE_CONFIG_HPP
#define ENGINE_ENGINE_CONFIG_HPP

#include "framework.hpp"

namespace NovaEngine
{
	struct GameWindowConfig
	{
		size_t minWidth;
		size_t minHeight;
		size_t maxWidth;
		size_t maxHeight;
		size_t width;
		size_t height;
		bool resizable;
		bool maximized;
		bool fullscreen;
		bool hidden;
	};

	struct EngineConfig
	{
		std::string name;
		GameWindowConfig window;
	};
};

#endif
