#include "Utility/SecretStore.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wincred.h>
#endif

namespace
{
const QString APP_NAMESPACE = QStringLiteral ("TranCE");
const QString FALLBACK_DIR_NAME = QStringLiteral ("secrets");

// wincred generic credentials are shared across logins sessions of the same
// Windows account; the fallback store is only protected by file permissions.
void logWarning (const QString &msg) { qWarning () << "[SecretStore]" << msg; }
} // namespace

bool SecretStore::isUsingSystemBackend ()
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

QString SecretStore::credentialName (const QString &key)
{
    return APP_NAMESPACE + "/" + key;
}

bool SecretStore::storeSecret (const QString &key, const QString &value)
{
    if (key.isEmpty ())
    {
        return false;
    }

#ifdef Q_OS_WIN
    if (storeSecretWincred (credentialName (key), value))
    {
        return true;
    }

    logWarning ("wincred store failed, falling back to file store");
#endif
    return storeSecretFile (key, value);
}

QString SecretStore::loadSecret (const QString &key)
{
    if (key.isEmpty ())
    {
        return QString ();
    }

#ifdef Q_OS_WIN
    const QString fromWincred =
        loadSecretWincred (credentialName (key));

    if (!fromWincred.isNull ())
    {
        return fromWincred;
    }
#endif
    return loadSecretFile (key);
}

bool SecretStore::deleteSecret (const QString &key)
{
    if (key.isEmpty ())
    {
        return false;
    }

    bool removed = false;

#ifdef Q_OS_WIN
    removed = deleteSecretWincred (credentialName (key));
#endif
    // Always attempt the file fallback too: a previous run may have written
    // it before the system backend became available.
    removed = deleteSecretFile (key) || removed;

    return removed;
}

#ifdef Q_OS_WIN

bool SecretStore::storeSecretWincred (const QString &name,
                                      const QString &value)
{
    const std::wstring wName = name.toStdWString ();
    const QByteArray bytes = value.toUtf8 ();

    CREDENTIALW credential = {};
    credential.Type = CRED_TYPE_GENERIC;
    credential.TargetName = const_cast<LPWSTR> (wName.c_str ());
    credential.CredentialBlobSize = static_cast<DWORD> (bytes.size ());
    credential.CredentialBlob =
        reinterpret_cast<LPBYTE> (const_cast<char *> (bytes.constData ()));
    credential.Persist = CRED_PERSIST_LOCAL_MACHINE;

    if (!CredWriteW (&credential, 0))
    {
        logWarning (QString ("CredWriteW failed: %1")
                        .arg (static_cast<long> (GetLastError ())));
        return false;
    }

    return true;
}

QString SecretStore::loadSecretWincred (const QString &name)
{
    PCREDENTIALW credential = nullptr;
    const std::wstring wName = name.toStdWString ();

    if (!CredReadW (wName.c_str (), CRED_TYPE_GENERIC, 0, &credential))
    {
        // ERROR_NOT_FOUND is the normal "key absent" case, not worth a log.
        return QString ();
    }

    QString value;

    if (credential->CredentialBlob != nullptr
        && credential->CredentialBlobSize > 0)
    {
        value = QString::fromUtf8 (
            reinterpret_cast<const char *> (credential->CredentialBlob),
            static_cast<int> (credential->CredentialBlobSize));
    }

    CredFree (credential);
    return value;
}

bool SecretStore::deleteSecretWincred (const QString &name)
{
    const std::wstring wName = name.toStdWString ();

    if (!CredDeleteW (wName.c_str (), CRED_TYPE_GENERIC, 0))
    {
        return false;
    }

    return true;
}

#endif // Q_OS_WIN

QString SecretStore::fallbackFilePath (const QString &key)
{
    const QString baseDir = QStandardPaths::writableLocation (
        QStandardPaths::AppDataLocation);

    // Hash the key so arbitrary caller key strings become safe file names.
    const QString hash = QString::fromLatin1 (
        QCryptographicHash::hash (key.toUtf8 (),
                                  QCryptographicHash::Sha256)
            .toHex ()
            .left (32));

    return baseDir + "/" + FALLBACK_DIR_NAME + "/" + hash + ".secret";
}

bool SecretStore::storeSecretFile (const QString &key, const QString &value)
{
    const QString path = fallbackFilePath (key);
    QDir dir = QFileInfo (path).dir ();

    if (!dir.mkpath ("."))
    {
        logWarning ("cannot create fallback secret directory: " + path);
        return false;
    }

    QSaveFile file (path);

    if (!file.open (QIODevice::WriteOnly))
    {
        logWarning ("cannot open fallback secret file: " + path);
        return false;
    }

    file.write (value.toUtf8 ());

    if (!file.commit ())
    {
        logWarning ("cannot commit fallback secret file: " + path);
        return false;
    }

    // Owner-only access; no-op semantics on Windows where this is only a
    // rare fallback path.
    QFile::setPermissions (path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    return true;
}

QString SecretStore::loadSecretFile (const QString &key)
{
    QFile file (fallbackFilePath (key));

    if (!file.open (QIODevice::ReadOnly))
    {
        return QString ();
    }

    return QString::fromUtf8 (file.readAll ());
}

bool SecretStore::deleteSecretFile (const QString &key)
{
    return QFile::remove (fallbackFilePath (key));
}
