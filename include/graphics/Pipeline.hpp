#ifndef ENGINE_GRAHICS_PIPELINE_HPP
#define ENGINE_GRAHICS_PIPELINE_HPP

#include "AbstractObject.hpp"

namespace NovaEngine::Graphics 
{
	class Context;

	class Pipeline : public AbstractObject<>
	{
		protected:
			bool onInitialize();
			bool onTerminate();

		private:
			Pipeline();

		friend class Context;
	};
};

#endif
