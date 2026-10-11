#include "Controller/AccountManager.h"
#include "Controller/DbManager.h"
#include "MainWindow.h"
#include "Model/UserModel.h"
#include "Utility/Logging.h"

#include <ElaApplication.h>
#include <QApplication>
#include <QDir>
#include <QMainWindow>
#include <QMessageBox>

#include <exception>

namespace
{
void
terminateHandler ()
{
  // Best-effort report before the process aborts; must not throw.
  try
    {
      throw;
    }
  catch (const std::exception &e)
    {
      trance::log::logException (lcApp, "Unhandled exception - terminating",
                                 e);
    }
  catch (...)
    {
      trance::log::logExceptionUnknown (lcApp,
                                        "Unhandled exception - terminating");
    }
}
}

int main (int argc, char *argv[])
{
    trance::log::installHandler ();
    std::set_terminate (terminateHandler);
    qCInfo (lcApp) << "TranCE starting";

    try
    {
        QApplication a (argc, argv);

        ElaApplication::getInstance ()->init ();

        auto &dbManager = DbManager::getInstance ();
        dbManager.initializeAsync ();

        MainWindow w;
        w.show ();

        AccountManager::getInstance ().setUserModel (&UserModel::getInstance ());
        return a.exec ();
    }
    catch (const std::exception &e)
    {
        trance::log::logException (lcApp,
                                   "Fatal error during startup or event loop",
                                   e);
        QMessageBox::critical (nullptr, QStringLiteral ("TranCE"),
                               QStringLiteral ("A fatal error occurred. "
                                               "See the log file for "
                                               "details:\n")
                                   + e.what ());
        return 1;
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (
            lcApp, "Fatal error during startup or event loop");
        QMessageBox::critical (
            nullptr, QStringLiteral ("TranCE"),
            QStringLiteral ("A fatal error occurred. See the log file for "
                            "details."));
        return 1;
    }
}
