#ifndef ENGINE_JOB_SYSTEM_HPP
#define ENGINE_JOB_SYSTEM_HPP

#include "framework.hpp"
#include "SubSystem.hpp"
#include "job_system/AtomicQueue.hpp"
#include "job_system/AtomicStack.hpp"

namespace NovaEngine::JobSystem
{
	class TaskManager : public SubSystem<size_t, size_t>
	{
	private:
		ENGINE_SUB_SYSTEM_CTOR(TaskManager) {  }

	protected:
		bool onInitialize(size_t maxJobs, size_t executionThreads);
		bool onTerminate();

	public:
	};
}

#endif
