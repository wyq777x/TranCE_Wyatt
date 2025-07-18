#pragma once
#include "Utility/CacheEntry.h"
#include <QCoreApplication>
#include <chrono>
#include <list>
#include <tuple>
#include <unordered_map>
#include <utility>


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

            move2Front (key);

            return it->second.value;
        }

        return std::nullopt;
    }

    void put (const std::string &key, T value,
              std::chrono::milliseconds ttl = std::chrono::minutes (5))
    {
        // Check if the key already exists
        auto it = cacheMap.find (key);
        if (it != cacheMap.end ())
        {
            // If it exists, update the value and refresh the expiration time
            it->second.value = std::move (value);
            it->second.refreshExpiration (ttl);
            move2Front (key);
            return;
        }

        // If the key does not exist, forward the value to emplace into the
        // cache
        auto entrySize = CacheEntry<T>::calculateSize (key, value);
        evict (entrySize);

        auto [iter, inserted] = cacheMap.emplace (
            std::piecewise_construct, std::forward_as_tuple (key),
            std::forward_as_tuple (key, std::move (value), ttl));

        if (inserted)
        {
            lruList.push_front (key);
            iteratorMap[key] = lruList.begin ();
            currentSize += entrySize;
        }

        return;
    }

    void remove (const std::string &key)
    {
        auto it = cacheMap.find (key);
        if (it != cacheMap.end ())
        {
            if (cleanupCallback)
            {
                cleanupCallback (it->second.value);
            }
            currentSize -= it->second.size;
            cacheMap.erase (it);
            auto iter_it = iteratorMap.find (key);
            if (iter_it != iteratorMap.end ())
            {
                lruList.erase (iter_it->second);
                iteratorMap.erase (iter_it);
            }
        }
    }

    void clear ()
    {
        cacheMap.clear ();
        lruList.clear ();
        iteratorMap.clear ();
        currentSize = 0;
        hitCount = 0;
        missCount = 0;
    }

    void cleanupExpired ()
    {
        for (auto it = cacheMap.begin (); it != cacheMap.end ();)
        {
            if (it->second.isExpired ())
            {
                std::string key = it->first;
                ++it;
                remove (key);
            }
            else
            {
                ++it;
            }
        }
    }

private:
    void move2Front (const std::string &key)
    {
        auto iter_it = iteratorMap.find (key);
        if (iter_it != iteratorMap.end ())
        {
            lruList.erase (iter_it->second);
            lruList.push_front (key);
            iter_it->second = lruList.begin ();
        }
        hitCount++;
    }

    void evict (std::size_t size)
    {
        while (currentSize + size >= maxSize && !lruList.empty ())
        {
            auto lastKey = lruList.back ();
            auto it = cacheMap.find (lastKey);
            if (it != cacheMap.end ())
            {
                if (cleanupCallback)
                {
                    cleanupCallback (it->second.value);
                }
                currentSize -= it->second.size;
                cacheMap.erase (it);
                iteratorMap.erase (lastKey);
                lruList.pop_back ();
            }
        }
        missCount++;
    }

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

    mutable std::size_t hitCount = 0;
    mutable std::size_t missCount = 0;

    mutable std::string m_lastError;

    template <typename ExceptionT>
    void logErr (const std::string &errMsg, const ExceptionT &e) const
    {
        qCritical () << "[CacheModel]" << QString::fromStdString (errMsg)
                     << "Exception:" << e.what ();
        m_lastError = errMsg + " Exception:" + std::string (e.what ());
    }
};