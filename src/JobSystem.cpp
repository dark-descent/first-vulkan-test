#include "JobSystem.hpp"

namespace NovaEngine
{
	void JobSystem::threadMain()
	{
		std::unique_lock<std::mutex> l(mutex_);
		cv_.wait(l, [&]() { return threadsRunning_; });
		l.unlock();

		size_t threadID = threadIdCounter_.fetch_add(1, std::memory_order::memory_order_release) + 1;
		cv_.notify_all();

		while (threadsRunning_)
		{
			runNextJob();
		}

		printf("thread done...\n");
	}

	void JobSystem::runNextJob()
	{
		Context* ctx = nullptr;
		readyContextqueue_.pop(&ctx);
		if (ctx != nullptr)
		{
			Job::Function f = reinterpret_cast<Job::Function>(ctx->rip);
			f(ctx->rbx);
		}
	}

	bool JobSystem::onInitialize(size_t maxJobs, size_t executionThreads)
	{
		threadCount_ = executionThreads;

		if (!readyContextqueue_.initialize(maxJobs))
			return  false;

		if (!freeContextStack_.initialize(maxJobs))
			return false;

		threadIdCounter_.store(0);

		for (size_t i = 0; i < maxJobs; i++)
			freeContextStack_.push(&contexts_[i]);

		for (size_t i = 0; i < threadCount_; i++)
			threads_.push_back(std::thread([&]() { threadMain(); }));

		return true;
	}

	bool JobSystem::onTerminate()
	{
		if (!threadsRunning_)
			startThreads(); // signal the threads to start and kill them peacefully
		stop();
		return true;
	}

	void JobSystem::startThreads()
	{
		if (!threadsRunning_)
		{
			threadsRunning_ = true;
			cv_.notify_all();

			std::unique_lock<std::mutex> l(mutex_);

			cv_.wait(l, [&]() { return threadIdCounter_.load() == threadCount_;	});

			l.unlock();
		}
	}

	void JobSystem::stop()
	{
		threadsRunning_ = false;
		for (size_t i = 0; i < threadCount_; i++)
			threads_[i].join();
	}

	Counter JobSystem::run(Job jobs[], size_t jobsCount)
	{
		Counter c = new CounterType(jobsCount);

		printf("%lu\n", freeContextStack_.size());

		for (size_t i = 0; i < jobsCount; i++)
		{
			Context* ctx = nullptr;
			freeContextStack_.pop(&ctx);

			if (ctx != nullptr)
			{
				ctx->rip = reinterpret_cast<void*>(jobs[i].function);
				ctx->rbx = jobs[i].args;
				readyContextqueue_.push(ctx);
			}
		}

		return c;
	}

	void JobSystem::waitForCounter(Counter c)
	{

	}

	void JobSystem::start()
	{
		while (readyContextqueue_.size() > 0)
		{
			runNextJob();
		}
	}
}

