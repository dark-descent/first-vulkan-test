#ifndef ENGINE_GRAHICS_PIPELINE_HPP
#define ENGINE_GRAHICS_PIPELINE_HPP

#include "AbstractObject.hpp"
#include "Engine.hpp"
#include "framework.hpp"

namespace NovaEngine::Graphics 
{
	class Context;

	class Pipeline : public AbstractObject<>
	{
		protected:
			bool onInitialize();
			bool onTerminate();

		private:
			Context* context_;
			Pipeline(Context*);

		public:
			VkShaderModule createShaderModule(const char* file);

		friend class Context;
	};
};

#endif
