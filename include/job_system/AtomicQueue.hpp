#ifndef ENGINE_JOB_SYSTEM_ATOMIC_QUEUE_HPP
#define ENGINE_JOB_SYSTEM_ATOMIC_QUEUE_HPP

#include "framework.hpp"

namespace NovaEngine::JobSystem
{
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

		size_t size() { return size_.load(std::memory_order::consume); }

		bool isEmpty() { return size() == 0; }

		bool push(T t)
		{
			int s = size_.fetch_add(1, std::memory_order::acq_rel);
			int i = lastIndex_.fetch_add(1, std::memory_order::acq_rel) % capacity_;

			if (s > static_cast<int>(capacity_ - 1))
			{
				size_.fetch_sub(1, std::memory_order::release);
				return false;
			}

			data_[i] = t;
			return true;
		}

		bool pop(T* t)
		{
			int s = size_.fetch_sub(1, std::memory_order::acq_rel);
			int i = firstIndex_.fetch_add(1, std::memory_order::acq_rel) % capacity_;
			if (s <= 0)
			{
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
