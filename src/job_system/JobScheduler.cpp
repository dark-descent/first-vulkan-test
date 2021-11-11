#include "job_system/JobScheduler.hpp"

namespace NovaEngine::JobSystem
{
	bool JobScheduler::onInitialize(size_t maxJobs, size_t executionThreads)
	{
		jobHandles_ = reinterpret_cast<JobHandle*>(malloc(sizeof(JobHandle) * maxJobs));
		jobQueue_.initialize(maxJobs);
		freeHandleStack_.initialize(maxJobs);
		waitList_.initialize(maxJobs);

		for (size_t i = 0; i < maxJobs; i++)
			freeHandleStack_.push(&jobHandles_[i]);

		return true;
	}

	bool JobScheduler::onTerminate()
	{
		free(jobHandles_);
		return true;
	}

	bool JobScheduler::runNextJob(JobHandlePtr* handleOut)
	{
		JobHandlePtr handlePtr = nullptr;
		jobQueue_.pop(&handlePtr);

		if (handlePtr == nullptr)
			return false;

		*handleOut = handlePtr;
		handlePtr->resume();
		return true;
	}

	Counter* JobScheduler::runJobs(JobInfo* jobs, size_t jobsCount)
	{
		Counter* c = new Counter(0);

		c->store(jobsCount, std::memory_order::relaxed);

		for (size_t i = 0; i < jobsCount; i++)
		{
			JobHandlePtr handlePtr = nullptr;
			freeHandleStack_.pop(&handlePtr);
			if (handlePtr != nullptr)
			{
				*handlePtr = jobs[i].function(c, this, jobs[i].arg);
				jobQueue_.push(handlePtr);
			}
		}

		return c;
	}

	void JobScheduler::exec()
	{
		JobHandlePtr jobHandle = nullptr;
		while (runNextJob(&jobHandle))
		{
			if (!jobHandle->done())
			{
				if (jobHandle->promise().state.isDone)
				{
					Counter* c = jobHandle->promise().state.counter;
					size_t index = c->fetch_sub(1, std::memory_order_acq_rel) - 1;
					if (index == 0)
					{
						// all dependecies with counter are finished!
						// move all from wait list to ready list where counter == 0
						// printf("all jobs with same counter done!\n");
						waitList_.findAndRelease(c, [&](JobHandlePtr handlePtr) {
							jobQueue_.push(handlePtr);
						});
					}
					jobHandle->destroy();
					freeHandleStack_.push(jobHandle);
				}
				else
				{
					// add to waiting list with counter dependency
					waitList_.push(jobHandle);
				}
			}
			else
			{
				jobHandle->destroy();
				freeHandleStack_.push(jobHandle);
			}
		}
	}
}
