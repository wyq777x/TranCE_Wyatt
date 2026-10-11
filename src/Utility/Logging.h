#pragma once

#include <QLoggingCategory>
#include <exception>

// Per-module logging categories. All log output of the app goes through one
// of these so it carries a stable "[module]" context and can be filtered at
// runtime via QT_LOGGING_RULES (e.g. "trance.*.debug=false\ntrance.db.debug=true").
Q_DECLARE_LOGGING_CATEGORY (lcApp)       // app lifecycle (main, global guards)
Q_DECLARE_LOGGING_CATEGORY (lcDb)        // Model/DbModel (SQLite persistence)
Q_DECLARE_LOGGING_CATEGORY (lcUser)      // Model/UserModel (profile files)
Q_DECLARE_LOGGING_CATEGORY (lcSettings)  // AppSettingModel + SettingManager
Q_DECLARE_LOGGING_CATEGORY (lcCache)     // CacheModel + CacheManager
Q_DECLARE_LOGGING_CATEGORY (lcAccount)   // AccountManager (auth flows)
Q_DECLARE_LOGGING_CATEGORY (lcDbMgr)     // DbManager facade
Q_DECLARE_LOGGING_CATEGORY (lcProvider)  // AiProviderManager
Q_DECLARE_LOGGING_CATEGORY (lcSidecar)   // AiSidecarManager (subprocess)
Q_DECLARE_LOGGING_CATEGORY (lcSecretStore) // SecretStore (API key storage)
Q_DECLARE_LOGGING_CATEGORY (lcView)      // View layer

namespace trance::log
{
// Installs the unified message handler: every message is written to stderr
// and appended to a size-rotated log file under the application data
// directory ("TranCE/logs/trance.log"). Must be called once, before any
// component logs - ideally as the first statement of main().
//
// The file location is deliberately built WITHOUT QCoreApplication
// application/organization names: the app never sets them, and setting them
// now would relocate SecretStore's QStandardPaths-based fallback files.
void installHandler ();

// A logging category passed around the Qt way: as the function declared by
// Q_DECLARE_LOGGING_CATEGORY (the qCDebug/qCCritical macros invoke it).
using CategoryFn = const QLoggingCategory &(*) ();

// Unified exception logging for call sites whose control flow must stay
// hand-written (coroutines, worker threads, rethrow boundaries).
void logException (CategoryFn cat, const char *op, const std::exception &e);
void logExceptionUnknown (CategoryFn cat, const char *op);
}
