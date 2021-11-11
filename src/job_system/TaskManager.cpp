#include "job_system/TaskManager.hpp"

namespace NovaEngine::JobSystem
{
	bool TaskManager::onInitialize(size_t maxJobs, size_t executionThreads)
	{
		return true;
	}

	bool TaskManager::onTerminate()
	{
		return true;
	}
}
