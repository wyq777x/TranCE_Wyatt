#pragma once

class AppController
{
public:
    static AppController &getInstance ()
    {
        static AppController instance;
        return instance;
    }

private:
    AppController () = default;
    AppController (const AppController &) = delete;
    AppController &operator= (const AppController &) = delete;
    AppController (AppController &&) = delete;
    AppController &operator= (AppController &&) = delete;
};