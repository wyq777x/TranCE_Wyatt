#pragma once

#include "Model/DbModel.h"

class DbManager
{

public:
    static DbManager &getInstance ()
    {
        static DbManager instance;
        return instance;
    }

private:
    DbManager () = default;
    DbManager (const DbManager &) = delete;
    DbManager &operator= (const DbManager &) = delete;
    DbManager (DbManager &&) = delete;
    DbManager &operator= (DbManager &&) = delete;
};