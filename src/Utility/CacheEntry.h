#pragma once
#include <chrono>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

struct CacheEntry
{
    std::string key;
    std::string value;
    std::chrono::system_clock::time_point insertTime;
    std::chrono::system_clock::time_point expirationTime;

    CacheEntry (std::string k, std::string v,
                std::chrono::system_clock::time_point exp)
        : key (std::move (k)), value (std::move (v)),
          insertTime (std::chrono::system_clock::now ()), expirationTime (exp)
    {
    }

    /*
bool isExpired () const
    {
        return std::chrono::system_clock::now () > expirationTime;
    }
  */
};

using Cache = std::unordered_map<std::string, CacheEntry>;
