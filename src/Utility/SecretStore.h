#pragma once

#include <QString>

/**
 * @brief Best-effort OS-backed secret storage for small secrets such as
 *        AI provider API keys.
 *
 * Backends (first available wins, per-operation fallback):
 *  - Windows: Credential Manager (wincred generic credentials).
 *  - Linux/other: file store under the application data directory with
 *    owner-only permissions (0600). This is a fallback, not a vault; the
 *    Secret Service integration is a planned upgrade behind the same
 *    interface.
 *
 * Keys are namespaced internally ("TranCE/<key>"), so arbitrary caller
 * key strings are safe.
 */
class SecretStore
{
public:
    // Returns true when the OS-backed backend is in use (as opposed to the
    // permission-based file fallback).
    static bool isUsingSystemBackend ();

    static bool storeSecret (const QString &key, const QString &value);

    // Returns empty string when the key does not exist or cannot be read.
    static QString loadSecret (const QString &key);

    static bool deleteSecret (const QString &key);

private:
    static QString credentialName (const QString &key);

    // wincred implementation (Windows only)
#ifdef Q_OS_WIN
    static bool storeSecretWincred (const QString &name, const QString &value);
    static QString loadSecretWincred (const QString &name);
    static bool deleteSecretWincred (const QString &name);
#endif

    // File fallback implementation
    static QString fallbackFilePath (const QString &key);
    static bool storeSecretFile (const QString &key, const QString &value);
    static QString loadSecretFile (const QString &key);
    static bool deleteSecretFile (const QString &key);
};
