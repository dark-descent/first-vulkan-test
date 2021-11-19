#ifndef ENGINE_GRAPHICS_COLOR_HPP
#define ENGINE_GRAPHICS_COLOR_HPP

#include "framework.hpp"

namespace NovaEngine::Graphics
{
	struct Color
	{
		float r;
		float g;
		float b;
		float a;

		Color(float r, float g, float b, float a = 1.0f) :
			r(r),
			g(g),
			b(b),
			a(a)
		{};
		Color() : Color(0.0f, 0.0f, 0.0f) {};

		VkClearColorValue getVkColor() { return { r, g, b, a }; }

		static Color black() { return Color(); }
		static Color white() { return Color(1.0f, 1.0f, 1.0f); }
		static Color red() { return Color(1.0f, 0.0f, 0.0f); }
		static Color blue() { return Color(0.0f, 1.0f, 0.0f); }
		static Color green() { return Color(0.0f, 0.0f, 1.0f); }
		static Color lightBlue() { return Color(0.0f, 1.0f, 1.0f); }
		static Color yello() { return Color(1.0f, 1.0f, 0.0f); }
		static Color pink() { return Color(1.0f, 0.0f, 1.0f); }
	};


};


#endif
