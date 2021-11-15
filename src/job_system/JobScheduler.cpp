#include "job_system/JobScheduler.hpp"

namespace NovaEngine::JobSystem
{
	bool JobScheduler::onInitialize(size_t maxJobs, size_t executionThreads)
	{
		maxJobs_ = maxJobs == 0 ? ENGINE_JOB_SYSTEM_MAX_JOBS : maxJobs;

		for (size_t i = 0; i < executionThreads; i++)
			threads_.push_back(std::thread([&] { threadEntry(); }));

		return true;
	}

	bool JobScheduler::onTerminate()
	{
		if (threadsRunning_.load(std::memory_order::acquire) == 1)
			stopThreads();

		for (std::thread& t : threads_)
			t.join();

		return true;
	}

	void JobScheduler::threadEntry()
	{
		std::thread::id threadID = std::this_thread::get_id();

		while (threadsRunning_.load(std::memory_order::acquire) != 1)
			; // wait (spin lock)

		printf("Thread with threadID %i started...\n", threadID);

		JobHandle jobHandle;
		while (threadsRunning_)
		{
			if (runNextJob(&jobHandle))
				handleJobYield(&jobHandle);
		}
	}

	bool JobScheduler::runNextJob(JobHandlePtr handleOut)
	{
		if (readyQueue_.pop(handleOut))
		{
			if (handleOut != nullptr && !handleOut->done())
			{
				handleOut->resume();
				return true;
			}
			return false;
		}

		return false;
	}

	Counter* JobScheduler::runJobs(JobInfo* jobs, size_t jobsCount)
	{
		Counter* c = new Counter(0);
		c->store(jobsCount, std::memory_order::relaxed);

		for (size_t i = 0; i < jobsCount; i++)
			readyQueue_.push(jobs[i].function(c, this, this->engine(), jobs[i].arg));

		return c;
	}

	Counter* JobScheduler::runJob(JobInfo job)
	{
		return runJobs(&job, 1);
	}

	Counter* JobScheduler::runJob(JobFunction function)
	{
		return runJob({ function, 0 });
	}

	void JobScheduler::execThreads()
	{
		threadsRunning_.store(1);
	}

	void JobScheduler::stopThreads()
	{
		threadsRunning_.store(0);
	}

	bool JobScheduler::handleJobYield(JobHandlePtr handle)
	{
		if (handle == nullptr)
		{
			return false;
		}
		else
		{
			std::unique_lock<std::mutex> s(jobYieldMutex_);

			auto [counter, isDone] = handle->promise().state;

			if (!isDone)
			{
				size_t c = counter->load(std::memory_order::seq_cst);
				if (c == 0)
				{
					readyQueue_.push(*handle);
				}
				else
				{
					while (!waitList_.pushWeak(*handle))
					{
						c = counter->load(std::memory_order::seq_cst);
						if (c == 0)
						{
							readyQueue_.push(*handle);
							return true;
						}
					}
				}
			}
			else
			{
				size_t index = counter->fetch_sub(1, std::memory_order::acq_rel) - 1;

				if (index == 0)
				{
					waitList_.findAndRelease([&](JobHandle handle) {
						if (handle.promise().state.counter == counter)
						{
							readyQueue_.push(handle);
							return true;
						}
						return false;
					});
					handle->destroy();
					
					delete counter;
				}
			}
		}
		return true;
	}
}
