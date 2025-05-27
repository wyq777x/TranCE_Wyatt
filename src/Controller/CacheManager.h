#pragma once
#include "Utility/CacheEntry.h"

class CacheManager
{
public:
    CacheManager () = default;
    ~CacheManager () = default;
    CacheManager (const std::string &cacheFile);

    CacheManager (const CacheManager &) = delete;
    CacheManager &operator= (const CacheManager &) = delete;
    CacheManager (CacheManager &&) = delete;
    CacheManager &operator= (CacheManager &&) = delete;

    template <typename T>

    T get (const std::string &key) const;

    void put (const std::string &key, const std::string &value);
    void applyCache ();
    void clearCache ();
    void saveCache ();
    void loadCache ();
};