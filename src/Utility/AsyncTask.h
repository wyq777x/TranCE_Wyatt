#pragma once

#include <coroutine>
#include <exception>

template <typename T> struct AsyncTask
{
    struct promise_type
    {
        T result;
        std::exception_ptr exception;

        std::suspend_never initial_suspend () { return {}; }
        std::suspend_always final_suspend () noexcept { return {}; }

        void return_value (T value) { result = std::move (value); }

        void return_void () { /* No-op for void return type */ }

        void unhandled_exception () { exception = std::current_exception (); }

        AsyncTask get_return_object ()
        {
            return AsyncTask{
                std::coroutine_handle<promise_type>::from_promise (*this)};
        }
    };
    std::coroutine_handle<promise_type> coro_handle;

    explicit AsyncTask (std::coroutine_handle<promise_type> handle)
        : coro_handle (handle)
    {
    }

    ~AsyncTask ()
    {
        if (coro_handle)
        {
            coro_handle.destroy ();
        }
    }
    T get ()
    {
        if (coro_handle.promise ().exception)
        {
            std::rethrow_exception (coro_handle.promise ().exception);
        }
        return std::move (coro_handle.promise ().result);
    }
};
