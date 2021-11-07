#ifndef ENGINE_GRAPHICS_GFX_OBJECT_HPP
#define ENGINE_GRAPHICS_GFX_OBJECT_HPP

#include "AbstractObject.hpp"

#define GFX_CTOR(type) friend class Context; type(Context* ctx) : GfxObject(ctx)

namespace NovaEngine::Graphics
{
	class Context;

	template<typename... Args>
	class GfxObject : AbstractObject<Args...>
	{
	protected:
		Context* ctx_;

		GfxObject(Context* ctx) : AbstractObject<Args...>(), ctx_(ctx) {  }

	public:
		Context* context() { return ctx_; };

		friend class Context;
	};
};

#endif
