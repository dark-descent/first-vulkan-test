#ifndef ENGINE_CONFIG_MANAGER_HPP
#define ENGINE_CONFIG_MANAGER_HPP

#include "framework.hpp"
#include "SubSystem.hpp"
#include "EngineConfig.hpp"

namespace NovaEngine
{
	class Engine;
	
	class ConfigManager : public SubSystem<v8::Global<v8::Object>*>
	{
	private:
		EngineConfig engineConfig_;
		bool isConfigured_;

		bool parseConfigObject(const v8::Local<v8::Object>& configuration);
		ENGINE_SUB_SYSTEM_CTOR(ConfigManager), engineConfig_(), isConfigured_(false) {}

	protected:
		bool onInitialize(v8::Global<v8::Object>* config);
		bool onTerminate();

	public:

		inline EngineConfig* getConfig() { return &engineConfig_; }
	};
};

#endif
