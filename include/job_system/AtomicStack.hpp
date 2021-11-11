#ifndef ENGINE_JOB_SYSTEM_ATOMIC_STACK_HPP
#define ENGINE_JOB_SYSTEM_ATOMIC_STACK_HPP

#include "framework.hpp"

namespace NovaEngine::JobSystem
{
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

		~AtomicStack()
		{
			if(data_ != nullptr)
				free(data_);
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

		size_t size() { return size_.load(std::memory_order::consume); }

		bool isEmpty() { return size() == 0; }

		bool push(T t)
		{
			int s = size_.fetch_add(1, std::memory_order::acq_rel);
			int i = stackPointer_.fetch_add(1, std::memory_order::acq_rel);

			if (s > static_cast<int>(capacity_ - 1))
			{
				printf("atomic stack overflow!!!\n");
				size_.fetch_sub(1, std::memory_order::release);
				return false;
			}

			data_[i] = t;

			return true;
		}

		bool pop(T* t)
		{
			int s = size_.fetch_sub(1, std::memory_order::acq_rel);
			int i = stackPointer_.fetch_sub(1, std::memory_order::acq_rel) - 1;

			if (s < 0)
			{
				printf("atomic stack underflow!!!\n");
				size_.fetch_add(1, std::memory_order::release);
				return false;
			}


			if (t != nullptr)
				*t = data_[i];

			return true;
		}

	};
};

#endif
