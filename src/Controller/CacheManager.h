#pragma once
#include "Utility/CacheEntry.h"

class CacheManager
{
public:
    static CacheManager &getInstance ()
    {
        static CacheManager instance;
        return instance;
    }

private:
    CacheManager () = default;
    CacheManager (const CacheManager &) = delete;
    CacheManager &operator= (const CacheManager &) = delete;
    CacheManager (CacheManager &&) = delete;
    CacheManager &operator= (CacheManager &&) = delete;
};