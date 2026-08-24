#include "Utility/PasswordHasher.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QStringList>
#include <cstring>

namespace
{
constexpr auto PASSWORD_SCHEME = "pbkdf2_sha256";

QByteArray toByteArray (const std::array<unsigned char, 32> &digest)
{
    return QByteArray (reinterpret_cast<const char *> (digest.data ()),
                       static_cast<int> (digest.size ()));
}
} // namespace

namespace PasswordHasherDetail
{
namespace
{
constexpr std::array<unsigned int, 64> K = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b,
    0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
    0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
    0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152,
    0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
    0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819,
    0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
    0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f,
    0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

inline unsigned int
rotr (unsigned int value, unsigned int count)
{
    return (value >> count) | (value << (32 - count));
}
} // namespace

Sha256::Sha256 ()
    : m_state{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
              0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19},
      m_bitLength (0), m_bufferSize (0)
{
}

void Sha256::update (const unsigned char *data, int length)
{
    m_bitLength += static_cast<unsigned long long> (length) * 8;

    while (length > 0)
    {
        const int fill = 64 - m_bufferSize;

        if (m_bufferSize > 0 || length < 64)
        {
            const int take = length < fill ? length : fill;

            std::memcpy (m_buffer + m_bufferSize, data, take);
            m_bufferSize += take;
            data += take;
            length -= take;

            if (m_bufferSize == 64)
            {
                processBlock (m_buffer);
                m_bufferSize = 0;
            }
        }
        else
        {
            // Fast path: whole blocks straight from the input.
            processBlock (data);
            data += 64;
            length -= 64;
        }
    }
}

void Sha256::update (const QByteArray &data)
{
    update (reinterpret_cast<const unsigned char *> (data.constData ()),
            data.size ());
}

void Sha256::processBlock (const unsigned char *block)
{
    unsigned int w[64];

    for (int i = 0; i < 16; ++i)
    {
        w[i] = (static_cast<unsigned int> (block[i * 4]) << 24)
               | (static_cast<unsigned int> (block[i * 4 + 1]) << 16)
               | (static_cast<unsigned int> (block[i * 4 + 2]) << 8)
               | static_cast<unsigned int> (block[i * 4 + 3]);
    }

    for (int i = 16; i < 64; ++i)
    {
        const unsigned int s0 =
            rotr (w[i - 15], 7) ^ rotr (w[i - 15], 18) ^ (w[i - 15] >> 3);
        const unsigned int s1 =
            rotr (w[i - 2], 17) ^ rotr (w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    unsigned int a = m_state[0];
    unsigned int b = m_state[1];
    unsigned int c = m_state[2];
    unsigned int d = m_state[3];
    unsigned int e = m_state[4];
    unsigned int f = m_state[5];
    unsigned int g = m_state[6];
    unsigned int h = m_state[7];

    for (int i = 0; i < 64; ++i)
    {
        const unsigned int S1 = rotr (e, 6) ^ rotr (e, 11) ^ rotr (e, 25);
        const unsigned int ch = (e & f) ^ (~e & g);
        const unsigned int temp1 = h + S1 + ch + K[i] + w[i];
        const unsigned int S0 = rotr (a, 2) ^ rotr (a, 13) ^ rotr (a, 22);
        const unsigned int maj = (a & b) ^ (a & c) ^ (b & c);
        const unsigned int temp2 = S0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;
    m_state[4] += e;
    m_state[5] += f;
    m_state[6] += g;
    m_state[7] += h;
}

std::array<unsigned char, 32> Sha256::finalize ()
{
    const unsigned long long bitLength = m_bitLength;

    // Append the 0x80 padding byte.
    const unsigned char padByte = 0x80;
    update (&padByte, 1);

    // The update() call above already grew m_bitLength, but the length
    // we append must be the pre-padding message length.
    while (m_bufferSize != 56)
    {
        const unsigned char zero = 0;
        update (&zero, 1);
    }

    unsigned char lengthBytes[8];

    for (int i = 0; i < 8; ++i)
    {
        lengthBytes[i] =
            static_cast<unsigned char> ((bitLength >> (56 - i * 8)) & 0xFF);
    }

    // Bypass update()'s bitLength bookkeeping for the final 8 bytes.
    std::memcpy (m_buffer + m_bufferSize, lengthBytes, 8);
    processBlock (m_buffer);
    m_bufferSize = 0;

    std::array<unsigned char, 32> digest;

    for (int i = 0; i < 8; ++i)
    {
        digest[i * 4] = static_cast<unsigned char> ((m_state[i] >> 24) & 0xFF);
        digest[i * 4 + 1] =
            static_cast<unsigned char> ((m_state[i] >> 16) & 0xFF);
        digest[i * 4 + 2] =
            static_cast<unsigned char> ((m_state[i] >> 8) & 0xFF);
        digest[i * 4 + 3] = static_cast<unsigned char> (m_state[i] & 0xFF);
    }

    return digest;
}

HmacSha256::HmacSha256 (const QByteArray &key)
{
    QByteArray normalizedKey = key;

    if (normalizedKey.size () > 64)
    {
        Sha256 keyHash;
        keyHash.update (key);
        normalizedKey = toByteArray (keyHash.finalize ());
    }

    unsigned char innerPad[64];
    unsigned char outerPad[64];

    for (int i = 0; i < 64; ++i)
    {
        const unsigned char byte = i < normalizedKey.size ()
                                       ? static_cast<unsigned char> (
                                           normalizedKey[i])
                                       : 0;

        innerPad[i] = byte ^ 0x36;
        outerPad[i] = byte ^ 0x5c;
    }

    m_inner.update (innerPad, 64);
    m_outer.update (outerPad, 64);
}

std::array<unsigned char, 32>
HmacSha256::operator() (const QByteArray &message) const
{
    // HMAC = H(opad || H(ipad || msg)); the pads are already absorbed
    // into the precomputed states, so each call only hashes the message.
    Sha256 inner = m_inner;
    inner.update (message);

    Sha256 outer = m_outer;
    outer.update (toByteArray (inner.finalize ()));

    return outer.finalize ();
}
} // namespace PasswordHasherDetail

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
    // The HMAC key (the password) never changes during derivation, so the
    // ipad/opad states are computed once and cloned per call instead of
    // being rebuilt 2*iterations times.
    const PasswordHasherDetail::HmacSha256 hmac (password);

    QByteArray derivedKey;
    derivedKey.reserve (keyLength);

    const int hashLength = 32;
    const int blockCount = (keyLength + hashLength - 1) / hashLength;

    for (int blockIndex = 1; blockIndex <= blockCount; ++blockIndex)
    {
        QByteArray block = toByteArray (hmac (
            salt + QByteArray (1, static_cast<char> (blockIndex)) +
            QByteArray (3, '\0')));
        QByteArray accumulator = block;

        // U_i = U_1 ^ U_2 ^ ... ^ U_i, where U_n = HMAC(password, U_{n-1}).
        for (int round = 1; round < iterations; ++round)
        {
            block = toByteArray (hmac (block));

            for (int i = 0; i < accumulator.size (); ++i)
            {
                accumulator[i] = static_cast<char> (accumulator[i] ^ block[i]);
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
