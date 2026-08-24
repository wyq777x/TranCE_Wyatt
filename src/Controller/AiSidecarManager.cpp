#include "Controller/AiSidecarManager.h"

#include "Controller/AccountManager.h"
#include "Controller/AiEventTap.h"
#include "Controller/AiProviderManager.h"
#include "Controller/DbManager.h"
#include "Controller/SettingManager.h"
#include "Model/DbModel.h"
#include "Utility/Constants.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcessEnvironment>
#include <QUuid>
#include <QUrl>
#include <vector>

namespace
{
constexpr int HEALTH_POLL_INTERVAL_MS = 300;
constexpr int START_TIMEOUT_MS = 30000; // python cold start can be slow
constexpr int MAX_RESTART_ATTEMPTS = 3;
constexpr const char *STDOUT_READY_TAG = "TRANCE_SIDECAR_READY ";

QString
findDevSidecarScript ()
{
    // Explicit override first, then the usual CMake build-directory layout:
    // <repo>/build/<preset>/<exe> -> <repo>/aisidecar/server/run.py
    const QStringList candidates = {
        qEnvironmentVariable ("TRANCE_AI_SOURCE_DIR")
            + "/aisidecar/server/run.py",
        QCoreApplication::applicationDirPath () + "/../../aisidecar/server/run.py"
    };

    for (const QString &candidate : candidates)
    {
        const QString canonical = QFileInfo (candidate).canonicalFilePath ();

        if (!canonical.isEmpty () && QFile::exists (canonical))
        {
            return canonical;
        }
    }

    return QString ();
}

QString
findVenvPython (const QString &scriptPath)
{
    // A virtualenv is identified by its pyvenv.cfg, so we don't blindly
    // trust a fixed relative path. Prefer the project's ".venv" (per the
    // sidecar README), then a sibling "venv", before falling back to the
    // system interpreter.
    const QDir scriptDir = QFileInfo (scriptPath).dir ();

    const QStringList venvDirs = {
        scriptDir.filePath (".venv"),
        scriptDir.filePath ("venv"),
    };

    for (const QString &dir : venvDirs)
    {
        if (!QFile::exists (QDir (dir).filePath ("pyvenv.cfg")))
        {
            continue;
        }

        const QString python = QDir (dir).filePath (
#ifdef Q_OS_WIN
            "Scripts/python.exe"
#else
            "bin/python"
#endif
        );

        if (QFile::exists (python))
        {
            return python;
        }
    }

    return QString ();
}
} // namespace

AiSidecarManager::AiSidecarManager ()
{
    m_nam = new QNetworkAccessManager (this);

    m_healthTimer.setInterval (HEALTH_POLL_INTERVAL_MS);
    m_healthTimer.setSingleShot (false);

    m_startTimeout.setInterval (START_TIMEOUT_MS);
    m_startTimeout.setSingleShot (true);

    connect (&m_healthTimer, &QTimer::timeout, this,
             &AiSidecarManager::pollHealth);
    connect (&m_startTimeout, &QTimer::timeout, this,
             [this] ()
             {
                 m_healthTimer.stop ();
                 scheduleRestart ("AI sidecar startup timed out");
             });

    connect (qApp, &QCoreApplication::aboutToQuit, this,
             &AiSidecarManager::shutdown);

    // Learning-behaviour signals -> learner model events. These are pure
    // no-ops unless the sidecar is running (offline guarantee).
    connect (&AiEventTap::getInstance (), &AiEventTap::quizAnswered, this,
             [this] (const QString &word, bool correct)
             {
                 QJsonObject event;
                 event["type"] = "quiz_answer";
                 event["word"] = word;
                 event["correct"] = correct;
                 forwardEvent (QJsonDocument (event).toJson (
                     QJsonDocument::Compact));
             });

    connect (&AiEventTap::getInstance (), &AiEventTap::wordLookedUp, this,
             [this] (const QString &word)
             {
                 QJsonObject event;
                 event["type"] = "lookup";
                 event["word"] = word;
                 forwardEvent (QJsonDocument (event).toJson (
                     QJsonDocument::Compact));
             });

    connect (&AiEventTap::getInstance (), &AiEventTap::wordRecited, this,
             [this] (const QString &word)
             {
                 QJsonObject event;
                 event["type"] = "recite";
                 event["word"] = word;
                 forwardEvent (QJsonDocument (event).toJson (
                     QJsonDocument::Compact));
             });

    connect (&AiEventTap::getInstance (), &AiEventTap::favoriteChanged,
             this,
             [this] (const QString &word, bool favorite)
             {
                 QJsonObject event;
                 event["type"] = "favorite";
                 event["word"] = word;
                 event["favorite"] = favorite;
                 forwardEvent (QJsonDocument (event).toJson (
                     QJsonDocument::Compact));
             });

    connect (&AiEventTap::getInstance (), &AiEventTap::wordStatusChanged,
             this,
             [this] (const QString &word, int status)
             {
                 QJsonObject event;
                 event["type"] = "word_status";
                 event["word"] = word;
                 event["status"] = status;
                 forwardEvent (QJsonDocument (event).toJson (
                     QJsonDocument::Compact));
             });
}

AiSidecarManager::~AiSidecarManager () { shutdown (); }

void AiSidecarManager::setState (State newState)
{
    if (m_state == newState)
    {
        return;
    }

    m_state = newState;
    emit stateChanged (newState);
}

QString AiSidecarManager::dataDir () const
{
    return QCoreApplication::applicationDirPath () + "/Utility/AI";
}

QString AiSidecarManager::baseUrl () const
{
    return m_port > 0 ? QString ("http://127.0.0.1:%1").arg (m_port)
                      : QString ();
}

QString AiSidecarManager::webEntryUrl () const
{
    if (m_port <= 0)
    {
        return QString ();
    }

    return baseUrl () + "/?token=" + m_token;
}

bool AiSidecarManager::resolveExecutable (QString &program,
                                          QStringList &args) const
{
    // 1. Explicit override.
    const QString override = qEnvironmentVariable ("TRANCE_AI_SIDECAR_EXE");

    if (!override.isEmpty () && QFile::exists (override))
    {
        program = override;
        return true;
    }

    // 2. Packaged one-directory build next to the application binary.
    const QString packaged =
        QCoreApplication::applicationDirPath () + "/aisidecar/aisidecar"
#ifdef Q_OS_WIN
        + ".exe"
#endif
        ;

    if (QFile::exists (packaged))
    {
        program = packaged;
        return true;
    }

    // 3. Development fallback: run the source tree with a python interpreter.
    const QString script = findDevSidecarScript ();

    if (!script.isEmpty ())
    {
        // Prefer the project virtualenv next to run.py so the sidecar's
        // dependencies (uvicorn/fastapi/...) are found without relying on
        // the system interpreter. $TRANCE_AI_PYTHON still wins if set.
        QString python = qEnvironmentVariable ("TRANCE_AI_PYTHON");

        if (python.isEmpty ())
        {
            python = findVenvPython (script);
        }

        if (python.isEmpty ())
        {
#ifdef Q_OS_WIN
            python = "python";
#else
            python = "python3";
#endif
        }

        program = python;
        // -u: unbuffered stdout so the READY line is not delayed.
        args << "-u" << script;
        return true;
    }

    return false;
}

void AiSidecarManager::ensureRunning ()
{
    if (m_state == State::Running || m_state == State::Starting)
    {
        if (m_state == State::Running)
        {
            // Refresh the session payload (e.g. after switching providers)
            // and re-announce readiness.
            pushSession ();
        }

        return;
    }

    // A manual (re)start from the UI gets a fresh restart budget, so Retry
    // works even after the automatic backoff has given up.
    m_restartAttempts = 0;

    spawnProcess ();
}

void AiSidecarManager::spawnProcess ()
{
    m_shutdownRequested = false;
    m_processOutput.clear ();

    if (!AccountManager::getInstance ().isLoggedIn ())
    {
        setState (State::Failed);
        emit failed (tr ("AI mode is bound to your profile: please log in "
                         "first."));
        return;
    }

    QString program;
    QStringList args;

    if (!resolveExecutable (program, args))
    {
        setState (State::Failed);
        emit failed (tr ("AI sidecar not found. Install the AI component "
                         "or set TRANCE_AI_SIDECAR_EXE."));
        return;
    }

    // Fresh one-time token per sidecar instance.
    m_token = QUuid::createUuid ().toString (QUuid::WithoutBraces)
              + QUuid::createUuid ().toString (QUuid::WithoutBraces);
    m_port = -1;

    args << "--port" << "0" // sidecar picks a free port, reports on stdout
         << "--data-dir" << dataDir ();

    QDir ().mkpath (dataDir ());

    if (m_process == nullptr)
    {
        m_process = new QProcess (this);

        connect (m_process, &QProcess::readyReadStandardOutput, this,
                 [this] ()
                 {
                     // The sidecar emits a single machine-readable line
                     // before any request logging.
                     const QByteArray out =
                         m_process->readAllStandardOutput ();

                     m_processOutput.append (out);

                     for (const QByteArray &line : out.split ('\n'))
                     {
                         if (line.startsWith (STDOUT_READY_TAG))
                         {
                             const QByteArray json =
                                 line.mid (qstrlen (STDOUT_READY_TAG));

                             const QJsonObject obj =
                                 QJsonDocument::fromJson (json).object ();

                             if (obj.contains ("port"))
                             {
                                 m_port = obj["port"].toInt ();
                                 m_healthTimer.start ();
                             }
                         }
                     }
                 });

        connect (m_process,
                 qOverload<int, QProcess::ExitStatus> (&QProcess::finished),
                 this,
                 [this] (int exitCode, QProcess::ExitStatus status)
                 {
                     m_healthTimer.stop ();
                     m_startTimeout.stop ();

                     if (m_shutdownRequested)
                     {
                         setState (State::NotRunning);
                         return;
                     }

                     QString reason =
                         QString ("AI sidecar exited unexpectedly (code %1, "
                                  "status %2)")
                             .arg (exitCode)
                             .arg (static_cast<int> (status));

                     // The Python sidecar writes its tracebacks to stderr,
                     // which is otherwise invisible. Surface the tail so the
                     // user (and we) can see why it died instead of a bare
                     // exit code.
                     QByteArray output = m_processOutput.trimmed ();

                     if (!output.isEmpty ())
                     {
                         reason += "\n" + QString::fromUtf8 (output);
                     }

                     scheduleRestart (reason);
                 });

        connect (m_process, &QProcess::errorOccurred, this,
                 [this] (QProcess::ProcessError error)
                 {
                     if (m_shutdownRequested)
                     {
                         return;
                     }

                     scheduleRestart (
                         QString ("AI sidecar process error (%1)")
                             .arg (static_cast<int> (error)));
                 });
    }

    QProcessEnvironment env =
        QProcessEnvironment::systemEnvironment ();
    env.insert ("TRANCE_AI_TOKEN", m_token);
    m_process->setProcessEnvironment (env);
    m_process->setProcessChannelMode (QProcess::MergedChannels);

    setState (State::Starting);
    m_process->start (program, args);
    m_startTimeout.start ();
}

void AiSidecarManager::pollHealth ()
{
    if (m_port <= 0)
    {
        return;
    }

    QNetworkRequest request (QUrl (baseUrl () + "/healthz"));
    QNetworkReply *reply = m_nam->get (request);

    connect (reply, &QNetworkReply::finished, this,
             [this, reply] ()
             {
                 const bool healthy =
                     reply->error () == QNetworkReply::NoError;

                 reply->deleteLater ();

                 if (healthy && m_state == State::Starting)
                 {
                     m_healthTimer.stop ();
                     m_startTimeout.stop ();
                     m_restartAttempts = 0;
                     setState (State::Running);
                     pushSession ();
                     pushSnapshot ();
                     emit ready (baseUrl ());
                 }
             });
}

void AiSidecarManager::pushSession ()
{
    if (m_port <= 0 || m_token.isEmpty ())
    {
        return;
    }

    QJsonObject provider; // stays null when none is configured yet

    const auto resolved =
        AiProviderManager::getInstance ().resolveActiveProvider (
            AccountManager::getInstance ().getUserUuid (
                AccountManager::getInstance ().getUsername ()));

    if (resolved.has_value ())
    {
        provider["base_url"] = resolved->config.baseUrl;
        provider["api_key"] = resolved->apiKey;
        provider["chat_model"] = resolved->config.chatModel;
        provider["embedding_model"] = resolved->config.embeddingModel;
    }

    QJsonObject session;
    session["user_uuid"] = AccountManager::getInstance ().getUserUuid (
        AccountManager::getInstance ().getUsername ());
    session["username"] = AccountManager::getInstance ().getUsername ();
    session["language"] = SettingManager::getInstance ().getLanguage ();
    session["provider"] = provider;

    // Read-only dictionary DB for the sidecar's native RAG corpus build.
    // The sidecar opens it with SQLite read-only mode; it never writes
    // application databases.
    session["dict_db_path"] =
        QDir (DbModel::getDbDir ()).filePath (Constants::Database::DICT_DB_NAME);

    QNetworkRequest request (QUrl (baseUrl () + "/api/session"));
    request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader ("Authorization", "Bearer " + m_token.toUtf8 ());

    QNetworkReply *reply = m_nam->post (
        request, QJsonDocument (session).toJson (QJsonDocument::Compact));

    connect (reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

void AiSidecarManager::pushSnapshot ()
{
    if (m_port <= 0 || m_token.isEmpty ())
    {
        return;
    }

    const QString userId = AccountManager::getInstance ().getUserUuid (
        AccountManager::getInstance ().getUsername ());

    if (userId.isEmpty ())
    {
        return;
    }

    auto toJsonArray = [] (const std::vector<QString> &words)
    {
        QJsonArray array;

        for (const QString &word : words)
        {
            array.append (word);
        }

        return array;
    };

    QJsonObject snapshot;
    snapshot["vocabulary_mastered"] = toJsonArray (
        DbManager::getInstance ().getUserVocabulary (userId, 1));
    snapshot["vocabulary_learning"] = toJsonArray (
        DbManager::getInstance ().getUserVocabulary (userId, 0));
    snapshot["favorites"] =
        toJsonArray (DbManager::getInstance ().getUserFavorites (userId));
    snapshot["recite_history"] = toJsonArray (
        DbManager::getInstance ().getUserReciteHistory (userId));
    snapshot["search_history"] = toJsonArray (
        DbManager::getInstance ().getUserSearchHistory (userId));

    QNetworkRequest request (QUrl (baseUrl () + "/api/sync/snapshot"));
    request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader ("Authorization", "Bearer " + m_token.toUtf8 ());

    QNetworkReply *reply = m_nam->post (
        request,
        QJsonDocument (snapshot).toJson (QJsonDocument::Compact));

    connect (reply, &QNetworkReply::finished, reply,
             &QNetworkReply::deleteLater);
}

void AiSidecarManager::forwardEvent (const QByteArray &eventJson)
{
    if (m_state != State::Running || m_port <= 0)
    {
        return;
    }

    QNetworkRequest request (QUrl (baseUrl () + "/api/sync/event"));
    request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader ("Authorization", "Bearer " + m_token.toUtf8 ());

    QNetworkReply *reply = m_nam->post (request, eventJson);

    connect (reply, &QNetworkReply::finished, reply,
             &QNetworkReply::deleteLater);
}

void AiSidecarManager::scheduleRestart (const QString &reason)
{
    if (m_shutdownRequested)
    {
        return;
    }

    if (m_restartAttempts >= MAX_RESTART_ATTEMPTS)
    {
        setState (State::Failed);
        emit failed (reason + tr (" - giving up after %1 attempts")
                             .arg (MAX_RESTART_ATTEMPTS));
        return;
    }

    qWarning () << "[AiSidecarManager]" << reason << "- restart attempt"
                << (m_restartAttempts + 1);

    setState (State::NotRunning);
    ++m_restartAttempts;

    // Simple linear backoff: 1s, 2s, 3s.
    QTimer::singleShot (m_restartAttempts * 1000, this,
                        [this] ()
                        {
                            if (!m_shutdownRequested
                                && m_state != State::Running)
                            {
                                spawnProcess ();
                            }
                        });
}

void AiSidecarManager::shutdown ()
{
    m_shutdownRequested = true;
    m_healthTimer.stop ();
    m_startTimeout.stop ();

    if (m_process == nullptr)
    {
        return;
    }

    if (m_process->state () != QProcess::NotRunning)
    {
        m_process->terminate ();

        if (!m_process->waitForFinished (2000))
        {
            m_process->kill ();
            m_process->waitForFinished (1000);
        }
    }

    setState (State::NotRunning);
}
