#include "AIPage.h"

#include "Controller/AiSidecarManager.h"
#include "Utility/Constants.h"
#include <ElaPushButton.h>
#include <QDesktopServices>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedLayout>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#ifdef TRANCE_AI_WEBENGINE
#include <QWebEngineView>
#endif

AIPage::AIPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("AI Mode"));

    initUI ();

    connect (&AiSidecarManager::getInstance (),
             &AiSidecarManager::stateChanged, this,
             &AIPage::onSidecarStateChanged);

    connect (&AiSidecarManager::getInstance (), &AiSidecarManager::ready,
             this, &AIPage::onSidecarReady);

    connect (&AiSidecarManager::getInstance (), &AiSidecarManager::failed,
             this, &AIPage::onSidecarFailed);
}

void AIPage::initUI ()
{
    QWidget *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("AI Mode"));

    QStackedLayout *stackedLayout =
        new QStackedLayout (centralWidget);

    // Status panel: shown while starting / on failure / as the browser
    // fallback.
    m_statusWidget = new QWidget (centralWidget);

    QVBoxLayout *statusLayout = new QVBoxLayout (m_statusWidget);
    statusLayout->setAlignment (Qt::AlignCenter);
    statusLayout->setSpacing (20);

    m_statusLabel = new QLabel (m_statusWidget);
    m_statusLabel->setAlignment (Qt::AlignCenter);
    m_statusLabel->setWordWrap (true);
    m_statusLabel->setFont (
        QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
               Constants::Settings::SUBTITLE_FONT_SIZE));
    m_statusLabel->setStyleSheet ("color: #666;");

    QHBoxLayout *buttonLayout = new QHBoxLayout ();
    buttonLayout->setAlignment (Qt::AlignCenter);
    buttonLayout->setSpacing (16);

    m_retryButton = new ElaPushButton (tr ("Retry"), m_statusWidget);
    m_retryButton->setMinimumSize (140, 44);

    m_openBrowserButton =
        new ElaPushButton (tr ("Open in Browser"), m_statusWidget);
    m_openBrowserButton->setMinimumSize (140, 44);

    buttonLayout->addWidget (m_retryButton);
    buttonLayout->addWidget (m_openBrowserButton);

    statusLayout->addWidget (m_statusLabel);
    statusLayout->addLayout (buttonLayout);

    stackedLayout->addWidget (m_statusWidget);

#ifdef TRANCE_AI_WEBENGINE
    m_webView = new QWebEngineView (centralWidget);
    stackedLayout->addWidget (m_webView);
#endif

    addCentralWidget (centralWidget, true, false, 0.0);

    connect (m_retryButton, &ElaPushButton::clicked, this,
             &AIPage::onRetryClicked);

    connect (m_openBrowserButton, &ElaPushButton::clicked, this,
             &AIPage::onOpenBrowserClicked);

    showStatus (tr ("AI mode is idle."), false, false);
}

void AIPage::activateAiMode ()
{
    auto &sidecar = AiSidecarManager::getInstance ();

    switch (sidecar.state ())
    {
    case AiSidecarManager::State::Running:
        loadWebUi ();
        break;

    default:
        showStatus (tr ("Starting AI engine..."), false, false);
        sidecar.ensureRunning ();
        break;
    }
}

void AIPage::onSidecarStateChanged (AiSidecarManager::State newState)
{
    if (newState == AiSidecarManager::State::Starting)
    {
        showStatus (tr ("Starting AI engine..."), false, false);
    }
}

void AIPage::onSidecarReady (const QString &baseUrl)
{
    Q_UNUSED (baseUrl);
    loadWebUi ();
}

void AIPage::onSidecarFailed (const QString &reason)
{
#ifdef TRANCE_AI_WEBENGINE
    showStatus (reason, true, false);
#else
    // Without the embedded engine, a running sidecar can still be used in
    // the system browser.
    showStatus (reason, true,
                AiSidecarManager::getInstance ().webEntryUrl ().isEmpty ()
                    ? false
                    : true);
#endif
}

void AIPage::onRetryClicked ()
{
    showStatus (tr ("Starting AI engine..."), false, false);
    AiSidecarManager::getInstance ().ensureRunning ();
}

void AIPage::onOpenBrowserClicked ()
{
    const QString url = AiSidecarManager::getInstance ().webEntryUrl ();

    if (!url.isEmpty ())
    {
        QDesktopServices::openUrl (QUrl (url));
        showStatus (tr ("AI mode is opened in your browser."), false, true);
    }
}

void AIPage::showStatus (const QString &text, bool showRetry,
                         bool showOpenBrowser)
{
    m_statusLabel->setText (text);
    m_retryButton->setVisible (showRetry);
    m_openBrowserButton->setVisible (showOpenBrowser);

    if (QStackedLayout *stack = qobject_cast<QStackedLayout *> (
            m_statusWidget->parentWidget ()->layout ()))
    {
        stack->setCurrentWidget (m_statusWidget);
    }
}

void AIPage::loadWebUi ()
{
    const QString url = AiSidecarManager::getInstance ().webEntryUrl ();

    if (url.isEmpty ())
    {
        return;
    }

#ifdef TRANCE_AI_WEBENGINE
    if (m_webView->url () != QUrl (url))
    {
        m_webView->load (QUrl (url));
    }

    m_webUiLoaded = true;

    if (QStackedLayout *stack = qobject_cast<QStackedLayout *> (
            m_webView->parentWidget ()->layout ()))
    {
        stack->setCurrentWidget (m_webView);
    }
#else
    showStatus (tr ("AI engine is running."), false, true);
#endif
}
