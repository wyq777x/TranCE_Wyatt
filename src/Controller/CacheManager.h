#pragma once
#include "Utility/CacheEntry.h"
#include <QCoreApplication>
#include <QDebug>

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

    mutable std::string m_lastError;

    template <typename ExceptionT>
    void logErr (const std::string &errMsg, const ExceptionT &e) const
    {
        qCritical () << "[CacheManager]"
                     << QString::fromStdString (errMsg).toUtf8 ().constData ()
                     << "Exception:" << e.what ();
        m_lastError = errMsg + " Exception:" + std::string (e.what ());
    }
};