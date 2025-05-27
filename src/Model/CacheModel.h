#pragma once
#include "Utility/CacheEntry.h"
#include <chrono>
#include <unordered_map>

template <typename T>

class CacheModel
{
public:
    CacheModel () = delete;

    CacheModel (std::size_t maxSize) : maxSize (maxSize) {}

    CacheModel (std::size_t maxSize,
                std::function<void (const T &)> cleanupCallback)
        : maxSize (maxSize), cleanupCallback (std::move (cleanupCallback))
    {
    }
    CacheModel (std::function<void (const T &)> cleanupCallback)
        : cleanupCallback (std::move (cleanupCallback))
    {
    }
    CacheModel (const CacheModel &) = delete;
    CacheModel (CacheModel &&) = default;
    CacheModel &operator= (const CacheModel &) = delete;
    CacheModel &operator= (CacheModel &&) = default;
    ~CacheModel () = default;

    void setCleanupCallback (std::function<void (const T &)> cleanupCallback)
    {
        this->cleanupCallback = std::move (cleanupCallback);
    }

    std::optional<T> get (const std::string &key)
    {
        auto it = cacheMap.find (key);
        if (it != cacheMap.end ())
        {
            if (it->second.isExpired ())
            {
                remove (key);
                return std::nullopt;
            }

            // move2Front(key);

            return it->second.value;
        }

        return std::nullopt;
    }

    void put (const std::string &key, T value,
              std::chrono::milliseconds ttl = std::chrono::minutes (5))
    {
    }

    void remove (const std::string &key) {}

    void clear () {}

    void cleanupExpired () {}

private:
    using CacheMap = std::unordered_map<std::string, CacheEntry<T>>;
    using LRUList = std::list<std::string>;
    using ListIterator = typename LRUList::iterator;
    using IteratorMap = std::unordered_map<std::string, ListIterator>;

    CacheMap cacheMap;
    LRUList lruList;
    IteratorMap iteratorMap;

    std::size_t maxSize = 1024 * 1024 * 10; // 10 MB default size
    std::size_t currentSize = 0;

    std::function<void (const T &)> cleanupCallback = nullptr;

    void move2Front (const std::string &key) {}

    void evict (std::size_t size) {}
};