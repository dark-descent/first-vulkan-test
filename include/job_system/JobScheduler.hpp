#ifndef ENGINE_JOB_SYSTEM_HPP
#define ENGINE_JOB_SYSTEM_HPP

#include "framework.hpp"
#include "SubSystem.hpp"
#include "job_system/AtomicQueue.hpp"
#include "job_system/AtomicStack.hpp"

#include <coroutine>
#include "job_system/Job.hpp"
#include "job_system/WaitList.hpp"

#define JOB(name) NovaEngine::JobSystem::Job name(NovaEngine::JobSystem::Counter* __COROOUTINE_COUNTER__, NovaEngine::JobSystem::JobScheduler* scheduler, void* arg)
#define awaitCounter(counter) co_yield { counter, false }
#define JOB_RETURN co_yield { __COROOUTINE_COUNTER__, true }

namespace NovaEngine::JobSystem
{
	class JobScheduler;


	typedef Job(*JobFunction)(Counter* counter, NovaEngine::JobSystem::JobScheduler* scheduler, void* arg);

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
		JobHandle* jobHandles_;
		AtomicQueue<JobHandle*> jobQueue_;
		AtomicStack<JobHandle*> freeHandleStack_;
		WaitList waitList_;

		ENGINE_SUB_SYSTEM_CTOR(JobScheduler), jobQueue_(), freeHandleStack_(), waitList_() {  }

	protected:
		bool onInitialize(size_t maxJobs, size_t executionThreads);
		bool onTerminate();
		bool runNextJob(JobHandlePtr* handleOut);

	public:
		Counter* runJobs(JobInfo* jobs, size_t jobsCount);

		void exec();
	};
}

#endif
