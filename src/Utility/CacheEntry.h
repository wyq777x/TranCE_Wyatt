#pragma once
#include <QByteArray>
#include <QPixmap>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

template <typename T>

struct CacheEntry
{
private:
    std::string key;
    T value;
    std::chrono::system_clock::time_point insertTime;
    std::chrono::system_clock::time_point expirationTime;
    std::size_t size = 0;

public:
    CacheEntry () = delete;
    CacheEntry (std::string key, T value,
                std::chrono::system_clock::time_point exp)
        : key (std::move (key)), value (std::move (value)),
          insertTime (std::chrono::system_clock::now ()), expirationTime (exp)
    {
        size = calculateSize (key, value);
    }

    CacheEntry (std::string key, T value,
                std::chrono::milliseconds ttl = std::chrono::minutes (5))
        : key (std::move (key)), value (std::move (value)),
          insertTime (std::chrono::system_clock::now ()),
          expirationTime (insertTime + ttl)
    {
        size = calculateSize (key, value);
    }

    bool isExpired () const
    {
        return std::chrono::system_clock::now () > expirationTime;
    }

    void refreshExpiration (std::chrono::milliseconds ttl)
    {
        expirationTime = std::chrono::system_clock::now () + ttl;
    }

    void refreshExpiration (std::chrono::system_clock::time_point exp)
    {
        expirationTime = exp;
    }

    static std::size_t calculateSize (const std::string &key, const T &value)
    {
        if constexpr (std::is_same_v<T, std::string>)
        {
            return key.size () + value.size ();
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<void>>)
        {
            return sizeof (T) + sizeof (key) + sizeof (value);
        }
        else if constexpr (std::is_same_v<T, QPixmap>)
        {
            return value.width () * value.height () *
                       4 + // Assuming 4 bytes per pixel (ARGB)
                   key.size ();
        }
        else if constexpr (std::is_same_v<T, QByteArray>)
        {
            return value.size () + key.size ();
        }
        else
        {
            return sizeof (T) + key.size ();
        }
    }
};
