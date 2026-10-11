#pragma once

#include "Utility/Logging.h"

#include <QElapsedTimer>
#include <QLoggingCategory>
#include <utility>

namespace trance::aspect
{
using trance::log::CategoryFn;

// ---------------------------------------------------------------------------
// Around-advice primitives: weave exception logging + fallback conversion +
// optional timing around a callable, replacing the hand-written
// "catch (SQLite::Exception) / catch (std::exception) / catch (...)" triples
// that used to be copied at every Model/Controller method boundary.
//
// On a std::exception (SQLite::Exception included - it derives from
// std::runtime_error) or an unknown exception, the exception is logged
// through the given category and `fallback` (or nothing) is returned,
// matching the historical "log and return default" contract.
//
// When the category has debug output enabled (off by default in production,
// toggleable at runtime via QT_LOGGING_RULES), successful invocations also
// report their elapsed time.
// ---------------------------------------------------------------------------

// For non-void callables. Pass `fallback` with an explicit type
// (e.g. std::optional<WordEntry>{}) so R is never deduced from a bare 0 or
// std::nullopt.
template <typename R, typename F>
R
guarded (CategoryFn cat, const char *op, R fallback, F &&fn)
{
  const bool wantTiming = cat ()->isDebugEnabled ();
  QElapsedTimer timer;
  if (wantTiming)
    timer.start ();
  try
    {
      R result = fn ();
      if (wantTiming)
        qCDebug (cat) << op << "took" << timer.elapsed () << "ms";
      return result;
    }
  catch (const std::exception &e)
    {
      trance::log::logException (cat, op, e);
    }
  catch (...)
    {
      trance::log::logExceptionUnknown (cat, op);
    }
  return fallback;
}

// For void callables.
template <typename F>
void
guarded (CategoryFn cat, const char *op, F &&fn)
{
  const bool wantTiming = cat ()->isDebugEnabled ();
  QElapsedTimer timer;
  if (wantTiming)
    timer.start ();
  try
    {
      fn ();
      if (wantTiming)
        qCDebug (cat) << op << "took" << timer.elapsed () << "ms";
    }
  catch (const std::exception &e)
    {
      trance::log::logException (cat, op, e);
    }
  catch (...)
    {
      trance::log::logExceptionUnknown (cat, op);
    }
}

// Logs any exception thrown by `fn` and rethrows it unchanged, for call
// sites whose contract is "log, then propagate" (rethrow boundaries,
// coroutines handing exceptions to AsyncTask::unhandled_exception).
template <typename F>
void
guardedRethrow (CategoryFn cat, const char *op, F &&fn)
{
  const bool wantTiming = cat ()->isDebugEnabled ();
  QElapsedTimer timer;
  if (wantTiming)
    timer.start ();
  try
    {
      fn ();
      if (wantTiming)
        qCDebug (cat) << op << "took" << timer.elapsed () << "ms";
    }
  catch (...)
    {
      try
        {
          throw;
        }
      catch (const std::exception &e)
        {
          trance::log::logException (cat, op, e);
        }
      catch (...)
        {
          trance::log::logExceptionUnknown (cat, op);
        }
      throw;
    }
}
} // namespace trance::aspect
