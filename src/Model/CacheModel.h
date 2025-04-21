#pragma once
#include <cstddef>
#include <string>
#include <unordered_map>

class CacheModel
{
public:
    CacheModel () = default;
    ~CacheModel () = default;

private:
    std::unordered_map<std::string, std::string> cacheMap;
    inline static std::size_t currentCacheSize = 0;
};