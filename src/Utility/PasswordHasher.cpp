#include "Utility/PasswordHasher.h"
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <QStringList>

namespace
{
constexpr auto PASSWORD_SCHEME = "pbkdf2_sha256";

QByteArray hmacSha256 (const QByteArray &key, const QByteArray &message)
{
    return QMessageAuthenticationCode::hash (message, key,
                                             QCryptographicHash::Sha256);
}

QByteArray buildBlockIndex (int blockIndex)
{
    QByteArray blockIndexBytes (4, Qt::Uninitialized);
    blockIndexBytes[0] = static_cast<char> ((blockIndex >> 24) & 0xFF);
    blockIndexBytes[1] = static_cast<char> ((blockIndex >> 16) & 0xFF);
    blockIndexBytes[2] = static_cast<char> ((blockIndex >> 8) & 0xFF);
    blockIndexBytes[3] = static_cast<char> (blockIndex & 0xFF);
    return blockIndexBytes;
}
} // namespace

QString PasswordHasher::hashPassword (const QString &password)
{
    const QByteArray salt = generateSalt ();
    const QByteArray derivedKey = deriveKeyPbkdf2Sha256 (
        password.toUtf8 (), salt, PBKDF2_ITERATIONS, DERIVED_KEY_SIZE);

    return QString ("%1$%2$%3$%4")
        .arg (PASSWORD_SCHEME)
        .arg (PBKDF2_ITERATIONS)
        .arg (QString::fromLatin1 (salt.toHex ()))
        .arg (QString::fromLatin1 (derivedKey.toHex ()));
}

bool PasswordHasher::verifyPassword (const QString &password,
                                     const QString &storedHash)
{
    if (storedHash.isEmpty ())
    {
        return false;
    }

    if (isLegacySha256Hash (storedHash))
    {
        const QByteArray legacyHash =
            QCryptographicHash::hash (password.toUtf8 (),
                                      QCryptographicHash::Sha256)
                .toHex ();
        return constantTimeEquals (legacyHash, storedHash.toLatin1 ());
    }

    const QStringList parts = storedHash.split ('$');
    if (parts.size () != 4 || parts[0] != PASSWORD_SCHEME)
    {
        return false;
    }

    bool iterationsOk = false;
    const int iterations = parts[1].toInt (&iterationsOk);
    if (!iterationsOk || iterations <= 0)
    {
        return false;
    }

    const QByteArray salt = QByteArray::fromHex (parts[2].toLatin1 ());
    const QByteArray expectedKey = QByteArray::fromHex (parts[3].toLatin1 ());

    if (salt.size () != SALT_SIZE || expectedKey.isEmpty ())
    {
        return false;
    }

    const QByteArray actualKey = deriveKeyPbkdf2Sha256 (
        password.toUtf8 (), salt, iterations, expectedKey.size ());

    return constantTimeEquals (actualKey, expectedKey);
}

bool PasswordHasher::needsRehash (const QString &storedHash)
{
    if (isLegacySha256Hash (storedHash))
    {
        return true;
    }

    const QStringList parts = storedHash.split ('$');
    if (parts.size () != 4 || parts[0] != PASSWORD_SCHEME)
    {
        return true;
    }

    bool iterationsOk = false;
    const int iterations = parts[1].toInt (&iterationsOk);
    const QByteArray salt = QByteArray::fromHex (parts[2].toLatin1 ());
    const QByteArray expectedKey = QByteArray::fromHex (parts[3].toLatin1 ());

    return !iterationsOk || iterations < PBKDF2_ITERATIONS ||
           salt.size () != SALT_SIZE ||
           expectedKey.size () != DERIVED_KEY_SIZE;
}

QByteArray PasswordHasher::generateSalt ()
{
    QByteArray salt (SALT_SIZE, Qt::Uninitialized);
    auto *generator = QRandomGenerator::system ();

    for (int i = 0; i < SALT_SIZE; ++i)
    {
        salt[i] = static_cast<char> (generator->bounded (256));
    }

    return salt;
}

QByteArray PasswordHasher::deriveKeyPbkdf2Sha256 (const QByteArray &password,
                                                  const QByteArray &salt,
                                                  int iterations,
                                                  int keyLength)
{
    QByteArray derivedKey;
    derivedKey.reserve (keyLength);

    const int hashLength = QCryptographicHash::hashLength (
        QCryptographicHash::Sha256);
    const int blockCount = (keyLength + hashLength - 1) / hashLength;

    for (int blockIndex = 1; blockIndex <= blockCount; ++blockIndex)
    {
        QByteArray block =
            hmacSha256 (password, salt + buildBlockIndex (blockIndex));
        QByteArray accumulator = block;

        for (int round = 1; round < iterations; ++round)
        {
            block = hmacSha256 (password, block);
            for (int i = 0; i < accumulator.size (); ++i)
            {
                accumulator[i] = static_cast<char> (
                    accumulator[i] ^ block[i]);
            }
        }

        derivedKey.append (accumulator);
    }

    derivedKey.truncate (keyLength);
    return derivedKey;
}

bool PasswordHasher::constantTimeEquals (const QByteArray &lhs,
                                         const QByteArray &rhs)
{
    if (lhs.size () != rhs.size ())
    {
        return false;
    }

    unsigned char diff = 0;
    for (int i = 0; i < lhs.size (); ++i)
    {
        diff |= static_cast<unsigned char> (lhs[i] ^ rhs[i]);
    }

    return diff == 0;
}

bool PasswordHasher::isLegacySha256Hash (const QString &storedHash)
{
    if (storedHash.size () != 64)
    {
        return false;
    }

    for (const QChar ch : storedHash)
    {
        if (!ch.isDigit () &&
            (ch.toLower () < QChar ('a') || ch.toLower () > QChar ('f')))
        {
            return false;
        }
    }

    return true;
}
