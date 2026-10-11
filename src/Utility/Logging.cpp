#include "Utility/Logging.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QtGlobal>

#include <cstdio>

Q_LOGGING_CATEGORY (lcApp, "trance.app")
Q_LOGGING_CATEGORY (lcDb, "trance.db")
Q_LOGGING_CATEGORY (lcUser, "trance.user")
Q_LOGGING_CATEGORY (lcSettings, "trance.settings")
Q_LOGGING_CATEGORY (lcCache, "trance.cache")
Q_LOGGING_CATEGORY (lcAccount, "trance.account")
Q_LOGGING_CATEGORY (lcDbMgr, "trance.dbmgr")
Q_LOGGING_CATEGORY (lcProvider, "trance.provider")
Q_LOGGING_CATEGORY (lcSidecar, "trance.sidecar")
Q_LOGGING_CATEGORY (lcSecretStore, "trance.secretstore")
Q_LOGGING_CATEGORY (lcView, "trance.view")

namespace trance::log
{
namespace
{
constexpr qint64 kMaxLogBytes = 5 * 1024 * 1024;
constexpr int kMaxBackups = 3;

QMutex g_mutex;
QFile g_logFile;
bool g_fileOpen = false;

QString
logFilePath ()
{
  // Built from AppDataLocation without an application name: the app never
  // sets one, and setting it now would relocate SecretStore's
  // QStandardPaths-based fallback secret files.
  const QString baseDir
      = QStandardPaths::writableLocation (QStandardPaths::AppDataLocation);
  if (baseDir.isEmpty ())
    return {};
  return baseDir + "/TranCE/logs/trance.log";
}

const char *
levelName (QtMsgType type)
{
  switch (type)
    {
    case QtDebugMsg:
      return "DEBUG";
    case QtInfoMsg:
      return "INFO";
    case QtWarningMsg:
      return "WARN";
    case QtCriticalMsg:
      return "CRIT";
    case QtFatalMsg:
      return "FATAL";
    }
  return "DEBUG";
}

bool
openLogFile ()
{
  const QString path = logFilePath ();
  if (path.isEmpty ())
    return false;
  if (!QDir ().mkpath (QFileInfo (path).absolutePath ()))
    return false;
  g_logFile.setFileName (path);
  if (!g_logFile.open (QIODevice::WriteOnly | QIODevice::Append
                       | QIODevice::Text))
    return false;
  g_fileOpen = true;
  return true;
}

void
rotateLogFile (const QString &path)
{
  // trance.log.2 -> .3, trance.log.1 -> .2, trance.log -> .1 (oldest dropped)
  for (int i = kMaxBackups; i >= 2; --i)
    {
      QFile::remove (QString ("%1.%2").arg (path).arg (i));
      QFile::rename (QString ("%1.%2").arg (path).arg (i - 1),
                     QString ("%1.%2").arg (path).arg (i));
    }
  QFile::remove (path + ".1");
  QFile::rename (path, path + ".1");
}

void
handleMessage (QtMsgType type, const QMessageLogContext &ctx,
               const QString &msg)
{
  // No qCxx() calls in here: the handler must never recurse into itself.
  QMutexLocker lock (&g_mutex);

  QString line
      = QStringLiteral ("[%1][%2][%3] %4")
            .arg (QDateTime::currentDateTime ().toString (
                      "yyyy-MM-dd hh:mm:ss.zzz"),
                  QLatin1String (levelName (type)),
                  QLatin1String (ctx.category ? ctx.category : "default"),
                  msg);
  if (ctx.file != nullptr)
    line += QStringLiteral (" (%1:%2)")
                .arg (QLatin1String (ctx.file))
                .arg (ctx.line);

  const QByteArray utf8 = line.toUtf8 ();
  std::fprintf (stderr, "%s\n", utf8.constData ());
  std::fflush (stderr);

  if (!g_fileOpen && !openLogFile ())
    return;
  if (g_logFile.size () + utf8.size () > kMaxLogBytes)
    {
      g_logFile.close ();
      g_fileOpen = false;
      rotateLogFile (g_logFile.fileName ());
      if (!openLogFile ())
        return;
    }
  g_logFile.write (utf8);
  g_logFile.write ("\n");
  g_logFile.flush ();
}
} // namespace

void
installHandler ()
{
  // Bare qDebug()/qWarning() printed before the refactor; keep trance.*
  // debug/info visible by default so the migration does not silence any
  // existing output. Explicit QT_LOGGING_RULES always win.
  if (qEnvironmentVariableIsEmpty ("QT_LOGGING_RULES"))
    QLoggingCategory::setFilterRules (
        QStringLiteral ("trance.*.debug=true\ntrance.*.info=true\n"));

  qInstallMessageHandler (handleMessage);
}

void
logException (CategoryFn cat, const char *op, const std::exception &e)
{
  qCCritical (cat) << op << "Exception:" << e.what ();
}

void
logExceptionUnknown (CategoryFn cat, const char *op)
{
  qCCritical (cat) << op << "Exception: Unknown exception";
}
} // namespace trance::log
