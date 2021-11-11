#ifndef ENGINE_JOB_SYSTEM_JOB_HPP
#define ENGINE_JOB_SYSTEM_JOB_HPP

#include "framework.hpp"
#include <coroutine>

namespace NovaEngine::JobSystem
{
	typedef std::atomic<size_t> Counter;

	struct Job
	{
		struct State
		{
			Counter* counter = nullptr;
			bool isDone = false;
		};

		struct promise_type
		{
			State state;
			promise_type() = default;
			Job get_return_object() { return { std::coroutine_handle<promise_type>::from_promise(*this) }; }
			std::suspend_always initial_suspend() { return {}; }
			std::suspend_never final_suspend() noexcept { return {}; }
			std::suspend_always yield_value(State s) { state = s; return {}; };
			void unhandled_exception() {}
		};

		std::coroutine_handle<promise_type> handle_;
		Job(std::coroutine_handle<promise_type> handle) : handle_(handle) {  }

		operator std::coroutine_handle<promise_type>() const { return handle_; }
	};

	typedef std::coroutine_handle<Job::promise_type> JobHandle;
	typedef JobHandle* JobHandlePtr;

};

#endif
