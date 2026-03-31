#pragma once

#include <QByteArray>
#include <QString>

class PasswordHasher
{
public:
    static QString hashPassword (const QString &password);
    static bool verifyPassword (const QString &password,
                                const QString &storedHash);
    static bool needsRehash (const QString &storedHash);

private:
    static constexpr int SALT_SIZE = 16;
    static constexpr int DERIVED_KEY_SIZE = 32;
    static constexpr int PBKDF2_ITERATIONS = 210000;

    static QByteArray generateSalt ();
    static QByteArray deriveKeyPbkdf2Sha256 (const QByteArray &password,
                                             const QByteArray &salt,
                                             int iterations,
                                             int keyLength);
    static bool constantTimeEquals (const QByteArray &lhs,
                                    const QByteArray &rhs);
    static bool isLegacySha256Hash (const QString &storedHash);
};
