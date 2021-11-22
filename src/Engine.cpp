#include "Engine.hpp"
#include "Logger.hpp"
#include "graphics/Color.hpp"

#define CHECK_REJECT(subSystem, rejector, msg) if(!subSystem) { rejector(msg); Logger::get()->error(#subSystem ":" #rejector " -> " msg); return false; }

namespace NovaEngine
{
	size_t frames = 0;
	GameWindow* win2 = nullptr;
	Graphics::Context* ctx2 = nullptr;



#pragma region Scripting Area
	namespace
	{
		v8::Global<v8::Promise::Resolver> configurePromiseResolver_;
		v8::Global<v8::Function> onLoadCallback_;
		v8::Global<v8::Object> configuredValue_;

		SCRIPT_METHOD(onEngineConfigure)
		{
			v8::Isolate* isolate_ = args.GetIsolate();
			v8::HandleScope handle_scope(isolate_);
			v8::Local<v8::Context> context = isolate_->GetCurrentContext();
			v8::Context::Scope context_scope(context);

			if (args.Length() < 1)
			{
				printf("Nein Nein Nein!\n");
			}
			else if (args[0]->IsObject())
			{
				v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(isolate_);
				v8::Local<v8::Promise> p = resolver->GetPromise();
				args.GetReturnValue().Set(p);
				configurePromiseResolver_.Reset(isolate_, resolver);
				configuredValue_.Reset(isolate_, args[0]->ToObject(isolate_));
			}
		}

		SCRIPT_METHOD(log)
		{
			v8::Isolate* isolate = args.GetIsolate();
			std::string buf = "[Game] ";
			for (int i = 0; i < args.Length(); i++)
			{
				auto a = args[i]->ToString(isolate);
				v8::String::Utf8Value val(a);
				buf += *val;
				if (i != args.Length() - 1)
					buf += ", ";
			}
			Logger::get()->info(buf);
		}

		// the game will call this to configure the engine
		SCRIPT_METHOD(onEngineLoad)
		{
			v8::Isolate* isolate_ = args.GetIsolate();
			v8::HandleScope handle_scope(isolate_);
			v8::Local<v8::Context> context = isolate_->GetCurrentContext();
			v8::Context::Scope context_scope(context);

			if (args.Length() < 1)
			{
				printf("Nein Nein Nein!\n");
			}
			else if (args[0]->IsFunction())
			{
				onLoadCallback_.Reset(isolate_, v8::Local<v8::Function>::Cast(args[0]));
			}
			args.GetReturnValue().Set(v8::Undefined(isolate_));
		}

		SCRIPT_METHOD(onEngineStart)
		{
			Engine* engine = ScriptManager::fetchEngineFromArgs(args);
			Logger::get()->info("on engine start called!");
			engine->start();
		}

		SCRIPT_METHOD(onShowWindow)
		{
			Engine* engine = ScriptManager::fetchEngineFromArgs(args);
			engine->gameWindow.show();
		}

		static void globalInitializer(ScriptManager* manager, const v8::Local<v8::Object>& o)
		{
			v8::Isolate* isolate = manager->isolate();
			v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

			v8::Local<v8::Object> engineObj = v8::Object::New(isolate);

			engineObj->Set(ctx, manager->createString("onLoad"), manager->createFunction(onEngineLoad));
			engineObj->Set(ctx, manager->createString("log"), manager->createFunction(log));
			engineObj->Set(ctx, manager->createString("start"), manager->createFunction(onEngineStart));

			v8::Local<v8::Object> windowObj = v8::Object::New(isolate);
			windowObj->Set(ctx, manager->createString("show"), manager->createFunction(onShowWindow));
			engineObj->Set(ctx, manager->createString("window"), windowObj);

			o->Set(ctx, manager->createString("Engine"), engineObj);
		};
	};
#pragma endregion

	char Engine::executablePath_[PATH_MAX];

	GameWindow* testW = nullptr;

	Engine::Engine() : AbstractObject(),
		isRunning_(false),
		assetManager(this),
		scriptManager(this),
		configManager(this),
		graphicsManager(this),
		jobScheduler(this),
		gameWindow(this)
	{
	}

	bool Engine::initSubSystem(const char* name, SubSystem<>* subSystem)
	{
		Logger* l = Logger::get();

		l->info("Initializing ", name, "...");
		if (!subSystem->initialize())
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

	Graphics::Context* ctx = nullptr;

	bool Engine::onInitialize(const char* gameStartupScript)
	{
		Logger::get()->info("Initializing Engine...");

		CHECK(initSubSystem("Asset manager", &assetManager, executablePath()), "Failed to initialize Asset Manager!");
		CHECK(initSubSystem("Script Manager", &scriptManager, globalInitializer), "Failed to initialie Script Manager!");

		scriptManager.load(gameStartupScript == nullptr ? "Game.js" : gameStartupScript);
		if (onLoadCallback_.IsEmpty())
			return false;

		bool configInitialized = false;

		// call Engine.onLoad() to provide a configure callback
		scriptManager.run([&](const ScriptManager::RunInfo& runInfo) {
			v8::Local<v8::Object> recv = v8::Object::New(runInfo.isolate);
			v8::Local<v8::Value> argv[] = { v8::Function::New(runInfo.isolate, onEngineConfigure) };
			onLoadCallback_.Get(runInfo.isolate)->Call(recv, 1, argv)->ToObject(runInfo.isolate);
		});

		auto rejectGameConfig = [&](const char* message) {
			scriptManager.run([&](const ScriptManager::RunInfo& runInfo) {
				v8::Local<v8::String> reason = v8::String::NewFromUtf8(runInfo.isolate, message, v8::NewStringType::kNormal).ToLocalChecked();
				configurePromiseResolver_.Get(runInfo.isolate)->Reject(reason);
			});
		};

		// now we wait till the callback from Engine.onLoad is done running
		// check if the script called the configure function 
		if (!configuredValue_.IsEmpty())
			configInitialized = initSubSystem("Config Manager", &configManager, &configuredValue_);

		CHECK_REJECT(configInitialized, rejectGameConfig, "Could not initialize Config Manager!");

		CHECK_REJECT(gameWindow.create(configManager.getConfig()->name.c_str(), configManager.getConfig()->window), rejectGameConfig, "Could not create Game Window!");

		Graphics::GraphicsConfig config = {};

		CHECK_REJECT(initSubSystem("Graphics Manager", &graphicsManager, &config), rejectGameConfig, "Could not create graphics stack!");

		CHECK_REJECT(jobScheduler.initialize(10000, std::thread::hardware_concurrency() - 1), rejectGameConfig, "Could not initialize Job System!");

		Graphics::SwapChainOptions scOptions = {
			.vSyncEnabled = true,
			.minFrames = 3,
		};

		Graphics::ContextOptions o = {
			.swapChainOptions = &scOptions,
			.clearColor = Graphics::Color(0.0f, 0.05f, 0.05f),
		};

		ctx = graphicsManager.createContext(gameWindow.glfwWindow(), &o);

		return true;
	}

	const char* Engine::executablePath()
	{
		if (executablePath_[0] == '\0')
		{
			int exePathLength = readlink("/proc/self/exe", executablePath_, PATH_MAX);
			if (exePathLength == -1)
			{
				Logger::get()->error("Could not set executablePath_!");
			}
			else
			{
				for (size_t i = exePathLength; i > 0; --i)
				{
					if (executablePath_[i] == '/')
					{
						executablePath_[i] = '\0';
						break;
					}
				}
			}
		}
		return executablePath_;
	}

	void Engine::run()
	{
		if (!configurePromiseResolver_.IsEmpty()) // probably first time started (with configuration passed) 
		{
			scriptManager.run([](const ScriptManager::RunInfo& runInfo) {
				configurePromiseResolver_.Get(runInfo.isolate)->Resolve(v8::Local<v8::Value>(v8::Undefined(runInfo.isolate)));
			});
			configurePromiseResolver_.Reset();
		}
	}

	bool Engine::isRunning()
	{
		return isRunning_;
	}

	JOB(pollEvents)
	{
		glfwPollEvents();
		scheduler->runJob(pollEvents);
		JOB_RETURN;
	}

	JOB(engineLoop)
	{
		GameWindow* w = static_cast<GameWindow*>(arg);

		ctx = engine->graphicsManager.getContextFromWindow(w);

		if (w->shouldClose())
		{
			engine->graphicsManager.destroyContext(ctx);
			w->destroy();
		}
		else
		{
			ctx->present([&]() {
				// when vsyn is on we can wait before we acquire the next image
				// this callback will be called every time the swapchain is not ready yet
				scheduler->execNext(); // lets execute the next job in the queue in the meanwhile 
			});

			scheduler->runJob({ engineLoop, w });

			std::cout << frames++ << std::endl;
		}
		
		JOB_RETURN;
	}


	void Engine::start()
	{
		if (!isRunning_)
		{
			Logger::get()->info("starting engine...");

			isRunning_ = true;

			win2 = new GameWindow(this);
			win2->create("test win 2", configManager.getConfig()->window);

			Graphics::SwapChainOptions scOptions = {};
			scOptions.minFrames = 3;
			scOptions.vSyncEnabled = true;

			Graphics::ContextOptions o2 = {
				.swapChainOptions = &scOptions,
				.clearColor = Graphics::Color(0.5f, 0.05f, 0.05f)
			};

			ctx2 = graphicsManager.createContext(win2->glfwWindow(), &o2);

			win2->show();

			JobSystem::JobInfo jobs[3] = {
				{ pollEvents },
				{ engineLoop, static_cast<void*>(&gameWindow) },
				{ engineLoop, static_cast<void*>(win2) },
			};

			jobScheduler.runJobs(jobs, 3);

			jobScheduler.exec([&] { return !gameWindow.isClosed(); }, [&] {
				// callback for each loop iteration
			});

			// if (win2 != nullptr)
			// 	win2->destroy();

			isRunning_ = false;
		}
	}

	void Engine::stop()
	{
		if (isRunning_)
		{
			isRunning_ = false;
		}
	}

	bool Engine::onTerminate()
	{
		configurePromiseResolver_.Reset();
		onLoadCallback_.Reset();
		configuredValue_.Reset();

		jobScheduler.terminate();
		graphicsManager.terminate();
		configManager.terminate();
		scriptManager.terminate();
		assetManager.terminate();

		Logger::terminate();

		glfwTerminate();

		return true;
	}
};
