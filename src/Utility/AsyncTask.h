#pragma once

#include <coroutine>
#include <exception>
#include <utility>

template <typename T>

struct AsyncTask
{
    struct promise_type
    {
        T result;
        std::exception_ptr exception;

        std::suspend_never initial_suspend () { return {}; }
        std::suspend_always final_suspend () noexcept { return {}; }

        void return_value (T value) { result = std::move (value); }

        void unhandled_exception () { exception = std::current_exception (); }

        AsyncTask get_return_object ()
        {
            return AsyncTask{
                std::coroutine_handle<promise_type>::from_promise (*this)};
        }
    };

    // Awaiter
    struct awaiter
    {
        std::coroutine_handle<promise_type> coro_handle;

        awaiter (std::coroutine_handle<promise_type> h) : coro_handle (h) {}

        bool await_ready () const noexcept { return coro_handle.done (); }

        std::coroutine_handle<>
        await_suspend (std::coroutine_handle<> awaiting_coro) noexcept
        {

            if (coro_handle.done ())
            {
                return awaiting_coro;
            }

            return coro_handle;
        }

        T await_resume ()
        {
            if (coro_handle.promise ().exception)
            {
                std::rethrow_exception (coro_handle.promise ().exception);
            }
            return std::move (coro_handle.promise ().result);
        }
    };

    std::coroutine_handle<promise_type> coro_handle;

    explicit AsyncTask (std::coroutine_handle<promise_type> handle)
        : coro_handle (handle)
    {
    }

    AsyncTask (AsyncTask &&other) noexcept
        : coro_handle (std::exchange (other.coro_handle, {}))
    {
    }

    AsyncTask &operator= (AsyncTask &&other) noexcept
    {
        if (this != &other)
        {
            if (coro_handle)
            {
                coro_handle.destroy ();
            }
            coro_handle = std::exchange (other.coro_handle, {});
        }
        return *this;
    }

    AsyncTask (const AsyncTask &) = delete;
    AsyncTask &operator= (const AsyncTask &) = delete;

    ~AsyncTask ()
    {
        if (coro_handle)
        {
            coro_handle.destroy ();
        }
    }

    awaiter operator co_await() { return awaiter{coro_handle}; }

    T get ()
    {
        if (coro_handle.promise ().exception)
        {
            std::rethrow_exception (coro_handle.promise ().exception);
        }
        return std::move (coro_handle.promise ().result);
    }

    bool is_ready () const noexcept
    {
        return coro_handle && coro_handle.done ();
    }
};

template <>

struct AsyncTask<void>
{
    struct promise_type
    {
        std::exception_ptr exception;

        std::suspend_never initial_suspend () { return {}; }
        std::suspend_always final_suspend () noexcept { return {}; }

        void return_void () {}

        void unhandled_exception () { exception = std::current_exception (); }

        AsyncTask get_return_object ()
        {
            return AsyncTask{
                std::coroutine_handle<promise_type>::from_promise (*this)};
        }
    };

    struct awaiter
    {
        std::coroutine_handle<promise_type> coro_handle;

        awaiter (std::coroutine_handle<promise_type> h) : coro_handle (h) {}

        bool await_ready () const noexcept { return coro_handle.done (); }

        std::coroutine_handle<>
        await_suspend (std::coroutine_handle<> awaiting_coro) noexcept
        {
            if (coro_handle.done ())
            {
                return awaiting_coro;
            }
            return coro_handle;
        }

        void await_resume ()
        {
            if (coro_handle.promise ().exception)
            {
                std::rethrow_exception (coro_handle.promise ().exception);
            }
        }
    };

    std::coroutine_handle<promise_type> coro_handle;

    explicit AsyncTask (std::coroutine_handle<promise_type> handle)
        : coro_handle (handle)
    {
    }

    AsyncTask (AsyncTask &&other) noexcept
        : coro_handle (std::exchange (other.coro_handle, {}))
    {
    }

    AsyncTask &operator= (AsyncTask &&other) noexcept
    {
        if (this != &other)
        {
            if (coro_handle)
            {
                coro_handle.destroy ();
            }
            coro_handle = std::exchange (other.coro_handle, {});
        }
        return *this;
    }

    AsyncTask (const AsyncTask &) = delete;
    AsyncTask &operator= (const AsyncTask &) = delete;

    ~AsyncTask ()
    {
        if (coro_handle)
        {
            coro_handle.destroy ();
        }
    }

    awaiter operator co_await() { return awaiter{coro_handle}; }

    void get ()
    {
        if (coro_handle.promise ().exception)
        {
            std::rethrow_exception (coro_handle.promise ().exception);
        }
    }

    bool is_ready () const noexcept
    {
        return coro_handle && coro_handle.done ();
    }
};
