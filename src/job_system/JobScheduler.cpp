#include "job_system/JobScheduler.hpp"

namespace NovaEngine::JobSystem
{
	bool JobScheduler::onInitialize(size_t maxJobs, size_t executionThreads)
	{
		
		return true;
	}

	bool JobScheduler::onTerminate()
	{
		return true;
	}
}
