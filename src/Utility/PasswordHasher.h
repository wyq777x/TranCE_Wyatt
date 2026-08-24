#pragma once

#include <QByteArray>
#include <QString>
#include <array>

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
    static constexpr int PBKDF2_ITERATIONS = 100000;

    static QByteArray generateSalt ();
    static QByteArray deriveKeyPbkdf2Sha256 (const QByteArray &password,
                                             const QByteArray &salt,
                                             int iterations,
                                             int keyLength);
    static bool constantTimeEquals (const QByteArray &lhs,
                                    const QByteArray &rhs);
    static bool isLegacySha256Hash (const QString &storedHash);
};

namespace PasswordHasherDetail
{
// Minimal SHA-256 with a copyable state, so PBKDF2 can precompute the
// key-dependent part of the two HMAC hashes once and then only run the
// cheap data rounds per iteration. This is what makes 100k iterations
// take milliseconds instead of the seconds a per-round
// QMessageAuthenticationCode construction costs.
class Sha256
{
public:
    Sha256 ();

    void update (const unsigned char *data, int length);
    void update (const QByteArray &data);
    std::array<unsigned char, 32> finalize ();

private:
    void processBlock (const unsigned char *block);

    std::array<unsigned int, 8> m_state;
    unsigned long long m_bitLength;
    unsigned char m_buffer[64];
    int m_bufferSize;
};

// HMAC-SHA256 with a reusable, copyable state: constructed once with the
// key, then copied and fed different messages without re-deriving the
// inner/outer pads.
class HmacSha256
{
public:
    explicit HmacSha256 (const QByteArray &key);

    std::array<unsigned char, 32>
    operator() (const QByteArray &message) const;

private:
    Sha256 m_inner;
    Sha256 m_outer;
};
} // namespace PasswordHasherDetail
