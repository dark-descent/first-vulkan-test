#ifndef ENGINE_JOB_SYSTEM_WAIT_LIST_HPP
#define ENGINE_JOB_SYSTEM_WAIT_LIST_HPP

#include "job_system/AtomicStack.hpp"
#include "job_system/Job.hpp"


namespace NovaEngine::JobSystem
{
	struct Job;

	class WaitList
	{
		AtomicStack<JobHandlePtr*> freeJobPtrs_;
		JobHandlePtr* data_;
		size_t capacity_;

	public:
		WaitList(size_t size = 0) : freeJobPtrs_(size), data_(nullptr), capacity_(size)
		{
			if (size != 0)
			{
				data_ = reinterpret_cast<JobHandlePtr*>(malloc(sizeof(JobHandlePtr) * size));
				memset(data_, 0, sizeof(JobHandlePtr) * size);
				for (size_t i = 0; i < size; i++)
					freeJobPtrs_.push(&data_[i]);
			}
		}

		~WaitList()
		{
			free(data_);
		}

		bool initialize(size_t size)
		{
			if (capacity_ == 0 && size != 0)
			{
				freeJobPtrs_.initialize(size);
				capacity_ = size;
				data_ = reinterpret_cast<JobHandlePtr*>(malloc(sizeof(JobHandlePtr) * size));
				memset(data_, 0, sizeof(JobHandlePtr) * size);
				for (size_t i = 0; i < size; i++)
					freeJobPtrs_.push(&data_[i]);
				return true;
			}
			return false;
		}

		bool push(JobHandlePtr jobHandle)
		{
			JobHandlePtr* handlePtr = nullptr;
			freeJobPtrs_.pop(&handlePtr);
			if (handlePtr != nullptr)
			{
				*handlePtr = jobHandle;
				return true;
			}

			return false;
		}

		template<typename Callback>
		void findAndRelease(Counter* counter, Callback callback)
		{
			for (size_t i = 0; i < capacity_; i++)
			{
				JobHandlePtr handle = data_[i];

				if (handle == nullptr)
					continue;

				if (handle->promise().state.counter == counter)
				{
					data_[i] = nullptr;
					callback(handle);
					freeJobPtrs_.push(&data_[i]);
				}
			}
		}
	};
}

#endif
