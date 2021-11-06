#ifndef ENGINE_ENGINE_HPP
#define ENGINE_ENGINE_HPP

#include "framework.hpp"
#include "ScriptManager.hpp"
#include "ConfigManager.hpp"
#include "AbstractObject.hpp"
#include "AssetManager.hpp"
#include "GameWindow.hpp"
#include "graphics/GraphicsManager.hpp"

namespace NovaEngine
{
	class Engine : public AbstractObject<const char*>
	{
	private:
		static char executablePath_[PATH_MAX];
		static std::vector<Terminatable*> subSystems_;

		bool isRunning_;

	public:
		AssetManager assetManager;
		ScriptManager scriptManager;
		ConfigManager configManager;
		Graphics::GraphicsManager graphicsManager;
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
		inline bool initSubSystem(const char* name, SubSystem<Args...>* subSystem, Args... args)
		{
			if (std::count(subSystems_.begin(), subSystems_.end(), subSystem))
			{
				printf("\033[;34m[%s]\033[0m:\033[;31m SubSystem is already initialized!\033[0m\n", name);
				return false;
			}
			else
			{
				printf("\033[;34m[%s]\033[0m: Initializing...\n", name);

				if (!subSystem->initialize(args...))
				{
					return false;
				}
				else
				{
					printf("\033[;34m[%s]\033[0m: Initialized!\n", name);
					subSystems_.push_back(subSystem);
					return true;
				}
			}
		}

		inline bool initSubSystem(const char* name, SubSystem<>* subSystem);
	};
};

#endif
