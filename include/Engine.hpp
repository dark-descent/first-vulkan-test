#ifndef ENGINE_ENGINE_HPP
#define ENGINE_ENGINE_HPP

#include "framework.hpp"
#include "ScriptManager.hpp"
#include "ConfigManager.hpp"
#include "AbstractObject.hpp"
#include "AssetManager.hpp"
#include "GameWindow.hpp"
#include "graphics/GraphicsManager.hpp"
#include "Logger.hpp"
#include "job_system/JobScheduler.hpp"

namespace NovaEngine
{
	class Engine : public AbstractObject<const char*>
	{
	private:
		static char executablePath_[PATH_MAX];

		bool isRunning_;

	public:
		AssetManager assetManager;
		ScriptManager scriptManager;
		ConfigManager configManager;
		Graphics::GraphicsManager graphicsManager;
		JobSystem::JobScheduler jobScheduler;
		GameWindow gameWindow;

		Engine();

		static const char* executablePath();

		bool isRunning();

		void run();

		/* gets called from the game to start and stop the engine */
		void start();
		/* gets called from the game to start and stop the engine */
		void stop();

	protected:

		bool onInitialize(const char*);
		bool onTerminate();

	private:

		template<typename... Args>
		bool initSubSystem(const char* name, SubSystem<Args...>* subSystem, Args... args)
		{
			Logger* l = Logger::get();

			l->info("Initializing ", name, "...");
			if (!subSystem->initialize(args...))
			{
				l->error("Failed to initialize ", name, "!");
				return false;
			}
			else
			{
				l->info(name, " initialized!");
				return true;
			}
		}

		inline bool initSubSystem(const char* name, SubSystem<>* subSystem);
	};
};

#endif
