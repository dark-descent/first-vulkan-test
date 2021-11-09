#ifndef ENGINE_JOB_SYSTEM_HPP
#define ENGINE_JOB_SYSTEM_HPP

#include "framework.hpp"
#include "SubSystem.hpp"

typedef std::atomic<size_t> CounterType;
typedef CounterType* Counter;
namespace NovaEngine
{
	struct Job
	{
		typedef void(*Function)(void* args);

		Function function = nullptr;
		void* args = nullptr;

		Job(Function function = nullptr, void* args = nullptr) : function(function), args(args)
		{

		}

		Job& operator=(Job& other)
		{
			this->function = other.function;
			this->args = other.args;
			return *this;
		}

		Job& operator=(Job&& other)
		{
			this->function = std::move(other.function);
			this->args = std::move(other.args);
			return *this;
		}
	};

	template<typename T>
	class AtomicStack
	{
		T* data_;
		size_t capacity_;
		std::atomic<int> stackPointer_;
		std::atomic<size_t> size_;

	public:
		AtomicStack(size_t size = 0) : capacity_(size)
		{
			data_ = size == 0 ? nullptr : reinterpret_cast<T*>(malloc(sizeof(T) * size));
			stackPointer_.store(0);
			size_.store(0);
		}

		bool initialize(size_t size)
		{
			if (data_ == nullptr && size != 0)
			{
				data_ = reinterpret_cast<T*>(malloc(sizeof(T) * size));
				capacity_ = size;
				return true;
			}
			return false;
		}

		size_t size() { return size_.load(std::memory_order::memory_order_consume); }

		bool isEmpty() { return size() == 0; }

		bool push(T t)
		{
			int s = size_.fetch_add(1, std::memory_order::memory_order_acq_rel);
			int i = stackPointer_.fetch_add(1, std::memory_order::memory_order_acq_rel);

			if (s > capacity_ - 1)
			{
				size_.fetch_sub(1, std::memory_order::memory_order_release);
				return false;
			}

			data_[i] = t;

			return true;
		}

		bool pop(T* t)
		{
			int s = size_.fetch_sub(1, std::memory_order::memory_order_acq_rel);
			// printf("size: %i\n", s);
			int i = stackPointer_.fetch_sub(1, std::memory_order::memory_order_acq_rel) - 1;

			if (s <= 0)
			{
				printf("size is smaller?? %i\n", s);

				size_.fetch_add(1, std::memory_order::memory_order_release);
				return false;
			}


			if (t != nullptr)
				*t = data_[i];

			return true;
		}

	};

	template<typename T>
	class AtomicQueue
	{
		T* data_;
		size_t capacity_;
		std::atomic<int> firstIndex_;
		std::atomic<int> lastIndex_;
		std::atomic<size_t> size_;
	public:

		AtomicQueue(size_t size = 0) : capacity_(size)
		{
			data_ = size == 0 ? nullptr : reinterpret_cast<T*>(malloc(sizeof(T) * size));
			firstIndex_.store(0);
			lastIndex_.store(0);
			size_.store(0);
		}

		bool initialize(size_t size)
		{
			if (data_ == nullptr && size != 0)
			{
				data_ = reinterpret_cast<T*>(malloc(sizeof(T) * size));
				capacity_ = size;
				return true;
			}
			return false;
		}

		size_t size() { return size_.load(std::memory_order::memory_order_consume); }

		bool isEmpty() { return size() == 0; }

		bool push(T t)
		{
			int s = size_.fetch_add(1, std::memory_order::memory_order_acq_rel);
			int i = lastIndex_.fetch_add(1, std::memory_order::memory_order_acq_rel) % capacity_;

			if (s > capacity_ - 1)
			{
				size_.fetch_sub(1, std::memory_order::memory_order_release);
				return false;
			}

			// printf("index: %i, size: %i\n", i, s);

			data_[i] = t;
			return true;
		}

		bool pop(T* t)
		{
			int s = size_.fetch_sub(1, std::memory_order::memory_order_acq_rel);
			int i = firstIndex_.fetch_add(1, std::memory_order::memory_order_acq_rel) % capacity_;

			if (s <= 0)
			{
				size_.fetch_add(1, std::memory_order::memory_order_release);
				return false;
			}

			// printf("load index: %i, size: %i\n", i, s);

			if (t != nullptr)
				*t = data_[i];

			return true;
		}
	};

	class JobSystem : SubSystem<size_t, size_t>
	{
	private:
		struct Context
		{
			Counter counter;
			void* rip;
			void* rsp;
			void* rbx;
			void* rbp;
			void* r12;
			void* r13;
			void* r14;
			void* r15;
		};

		std::mutex mutex_;
		std::condition_variable cv_;


		std::atomic<size_t> threadIdCounter_;
		std::vector<std::thread> threads_;
		size_t threadCount_;
		bool threadsRunning_;

		Context* contexts_;

		AtomicStack<Context*> freeContextStack_;
		AtomicQueue<Context*> readyContextqueue_;

		const std::thread::id MAIN_THREAD_ID = std::this_thread::get_id();

		void threadMain();

		void runNextJob();

		ENGINE_SUB_SYSTEM_CTOR(JobSystem),
			mutex_(),
			cv_(),
			threadIdCounter_(),
			threads_(),
			threadCount_(0),
			threadsRunning_(false),
			contexts_(nullptr)
		{}

	protected:
		bool onInitialize(size_t maxJobs, size_t executionThreads);
		bool onTerminate();

	public:
		void startThreads();
		void stop();
		Counter run(Job jobs[], size_t jobsCount);
		void waitForCounter(Counter c);
		void start();
	};
}

#endif
