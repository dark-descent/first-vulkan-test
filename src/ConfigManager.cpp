#include "ConfigManager.hpp"

#include "framework.hpp"
#include "EngineConfig.hpp"
#include "Engine.hpp"
#include "ScriptManager.hpp"
#include "GameWindow.hpp"

namespace NovaEngine
{
	namespace
	{
		namespace Parser
		{
			bool isUndefined(v8::Local<v8::Object> obj, const char* name)
			{
				return obj->Get(v8::String::NewFromUtf8(obj->CreationContext()->GetIsolate(), name))->IsUndefined();
			}

			v8::Local<v8::Object> parseObj(v8::Local<v8::Object> obj, const char* name, uint32_t defaultValue = 0)
			{
				return obj->Get(v8::String::NewFromUtf8(obj->CreationContext()->GetIsolate(), name))->ToObject();
			}

			uint32_t parseUint(v8::Local<v8::Object> obj, const char* name, uint32_t defaultValue = 0)
			{
				v8::Local<v8::Value> val = obj->Get(v8::String::NewFromUtf8(obj->CreationContext()->GetIsolate(), name));
				if (val->IsUndefined())
					return defaultValue;
				return val->Uint32Value();
			}

			int32_t parseInt(v8::Local<v8::Object> obj, const char* name, int32_t defaultValue = 0)
			{
				v8::Local<v8::Value> val = obj->Get(v8::String::NewFromUtf8(obj->CreationContext()->GetIsolate(), name));
				if (val->IsUndefined())
					return defaultValue;
				return val->Int32Value();
			}

			std::string parseString(v8::Local<v8::Object> obj, const char* name, const char* defaultValue = "")
			{
				v8::Local<v8::Value> val = obj->Get(v8::String::NewFromUtf8(obj->CreationContext()->GetIsolate(), name));

				if (val->IsUndefined())
					return std::string(defaultValue);

				v8::String::Utf8Value utf8Val(val->ToString());
				return std::string(*utf8Val);
			}

			bool parseBool(v8::Local<v8::Object> obj, const char* name, bool defaultValue = false)
			{
				v8::Local<v8::Value> val = obj->Get(v8::String::NewFromUtf8(obj->CreationContext()->GetIsolate(), name));

				if (val->IsUndefined())
					return defaultValue;
				return val->BooleanValue();
			}
		};

	};

	bool ConfigManager::parseConfigObject(const v8::Local<v8::Object>& config)
	{
		if (!isConfigured_)
		{
			using namespace v8;

			engineConfig_.name = Parser::parseString(config, "name", "MISSING NAME");

			if (!Parser::isUndefined(config, "window"))
			{
				Local<Object> windowObj = Parser::parseObj(config, "window");
				engineConfig_.window.minWidth = Parser::parseUint(windowObj, "minWidth", 640);
				engineConfig_.window.maxWidth = Parser::parseUint(windowObj, "maxWidth", std::numeric_limits<uint32_t>::max());
				engineConfig_.window.minHeight = Parser::parseUint(windowObj, "minHeight", 480);
				engineConfig_.window.maxHeight = Parser::parseUint(windowObj, "maxHeight", std::numeric_limits<uint32_t>::max());
				engineConfig_.window.height = Parser::parseUint(windowObj, "height", 480);
				engineConfig_.window.width = Parser::parseUint(windowObj, "width", 640);
				engineConfig_.window.resizable = Parser::parseBool(windowObj, "resizable", false);
				engineConfig_.window.fullscreen = Parser::parseBool(windowObj, "fullscreen", false);
				engineConfig_.window.maximized = Parser::parseBool(windowObj, "maximized", true);
				engineConfig_.window.hidden = Parser::parseBool(windowObj, "hidden", false);
			}
			
			ScriptManager::printObject(config->CreationContext()->GetIsolate(), config, "config");
			isConfigured_ = true;			
			return true;
		}

		return false;
	}

	bool ConfigManager::onInitialize(v8::Global<v8::Object>* config)
	{
		if (!isConfigured_)
			engine()->scriptManager.run([&](ScriptManager::RunInfo& info) {
				isConfigured_ = parseConfigObject(config->Get(info.isolate)->ToObject(info.isolate));
			});
		
		return isConfigured_;
	}

	bool ConfigManager::onTerminate()
	{
		return true;
	}
};
