#ifndef ENGINE_JOB_SYSTEM_HPP
#define ENGINE_JOB_SYSTEM_HPP

#include "framework.hpp"
#include "SubSystem.hpp"

#include <coroutine>
#include "job_system/Job.hpp"
#include "job_system/Queue.hpp"
#include "job_system/WaitList.hpp"

#define JOB(name) NovaEngine::JobSystem::Job name(NovaEngine::JobSystem::Counter* __COROUTINE_COUNTER__, NovaEngine::JobSystem::JobScheduler* scheduler, NovaEngine::Engine* engine, void* arg)
#define awaitCounter(counter) co_yield { counter, false }
#define JOB_RETURN co_yield { __COROUTINE_COUNTER__, true }

#ifndef ENGINE_JOB_SYSTEM_MAX_JOBS
#define ENGINE_JOB_SYSTEM_MAX_JOBS 200
#endif


namespace NovaEngine::JobSystem
{
	class JobScheduler;

	typedef Job(*JobFunction)(Counter* counter, NovaEngine::JobSystem::JobScheduler* scheduler, NovaEngine::Engine* engine, void* arg);

	struct JobInfo
	{
		JobFunction function = nullptr;
		void* arg = nullptr;

		template<typename T>
		JobInfo(JobFunction function, T arg) : function(function), arg(reinterpret_cast<void*>(arg)) {}

		JobInfo(JobFunction function = nullptr, void* arg = nullptr) : function(function), arg(arg) {}
	};

	class JobScheduler : public SubSystem<size_t, size_t>
	{
	private:
		size_t maxJobs_;
		Queue<JobHandle> readyQueue_;
		List<JobHandle> waitList_;
		std::mutex jobYieldMutex_;

		std::vector<std::thread> threads_;
		std::thread::id mainThreadID_;
		std::atomic<int> threadsRunning_;

		ENGINE_SUB_SYSTEM_CTOR(JobScheduler),
			maxJobs_(ENGINE_JOB_SYSTEM_MAX_JOBS),
			readyQueue_(),
			waitList_(),
			jobYieldMutex_(),
			threads_(),
			mainThreadID_(std::this_thread::get_id()),
			threadsRunning_()
		{ threadsRunning_.store(0); }

	protected:
		bool onInitialize(size_t maxJobs, size_t executionThreads);
		bool onTerminate();
		bool runNextJob(JobHandle* handleOut);
		void threadEntry();
		bool handleJobYield(JobHandle* handle);

	public:
		Counter* runJobs(JobInfo* jobs, size_t jobsCount);
		Counter* runJob(JobInfo jobs);
		Counter* runJob(JobFunction func);

		template<typename LoopConditionCallback, typename LoopCallback>
		void exec(LoopConditionCallback shouldLoop, LoopCallback loopCallback = []() {})
		{
			if (mainThreadID_ != std::this_thread::get_id())
				throw std::runtime_error("Cannot call JobScheduler::exec() from another thread than the main thread!");

			if (threadsRunning_.load() == 0)
				execThreads();

			JobHandle jobHandle;

			while (shouldLoop())
			{
				if (runNextJob(&jobHandle))
					handleJobYield(&jobHandle);
				loopCallback();
			}

		}

		void execThreads();
		void stopThreads();
	};
}

#endif
