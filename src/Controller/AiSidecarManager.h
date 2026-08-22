#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>

class QNetworkAccessManager;
class QNetworkReply;

/**
 * @brief Owns the lifetime of the AI sidecar process (Python/FastAPI).
 *
 * The sidecar is spawned lazily when AI mode is opened and torn down on
 * application exit or logout. Communication happens over localhost HTTP
 * only; every request must carry the one-time bearer token generated here
 * and handed to the process through its environment (not the command
 * line, which is visible to other local processes).
 *
 * Startup protocol:
 *  1. spawn with TRANCE_AI_TOKEN + --port 0 in the environment/args
 *  2. sidecar picks a free port and prints one JSON line to stdout:
 *       TRANCE_SIDECAR_READY {"port": 12345}
 *  3. this manager polls GET /healthz, then pushes POST /api/session with
 *     the resolved provider credentials
 *  4. emits ready(baseUrl)
 *
 * The executable is resolved in this order:
 *  1. $TRANCE_AI_SIDECAR_EXE (absolute path, dev override)
 *  2. <appDir>/aisidecar/aisidecar[.exe] (PyInstaller onedir, release)
 *  3. dev fallback: $TRANCE_AI_PYTHON (or "python") running
 *     <repo>/aisidecar/server/run.py next to the binary's source tree
 */
class AiSidecarManager : public QObject
{
    Q_OBJECT

public:
    enum class State
    {
        NotRunning,
        Starting,
        Running,
        Failed
    };
    Q_ENUM (State)

    static AiSidecarManager &getInstance ()
    {
        static AiSidecarManager instance;
        return instance;
    }

    AiSidecarManager (const AiSidecarManager &) = delete;
    AiSidecarManager &operator= (const AiSidecarManager &) = delete;

    // Idempotent. When the sidecar is already running the ready signal is
    // re-emitted immediately (after refreshing the session payload).
    void ensureRunning ();

    void shutdown ();

    State state () const { return m_state; }

    QString baseUrl () const;

    // The web entry URL embeds the token so the bundled web UI can
    // authenticate its API calls.
    QString webEntryUrl () const;

signals:
    void stateChanged (AiSidecarManager::State newState);
    void ready (const QString &baseUrl);
    void failed (const QString &reason);

private:
    explicit AiSidecarManager ();
    ~AiSidecarManager () override;

    void setState (State newState);

    bool resolveExecutable (QString &program, QStringList &args) const;
    void spawnProcess ();
    void pollHealth ();
    void pushSession ();
    void pushSnapshot ();
    void forwardEvent (const QByteArray &eventJson);
    void scheduleRestart (const QString &reason);

    QString dataDir () const;

    QNetworkAccessManager *m_nam = nullptr;
    QProcess *m_process = nullptr;
    QTimer m_healthTimer;
    QTimer m_startTimeout;

    State m_state = State::NotRunning;
    QString m_token;
    int m_port = -1;
    int m_restartAttempts = 0;
    bool m_shutdownRequested = false;
};
