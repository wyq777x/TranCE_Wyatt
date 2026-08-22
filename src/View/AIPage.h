#pragma once

#include "Controller/AiSidecarManager.h"
#include "TempPage.h"
#include <QWidget>

class QLabel;
class ElaPushButton;

#ifdef TRANCE_AI_WEBENGINE
class QWebEngineView;
#endif

/**
 * @brief Hosts the AI mode experience served by the local sidecar.
 *
 * With QtWebEngine available the web UI is embedded; otherwise the page
 * degrades to a status panel with an "open in browser" action. The sidecar
 * itself is owned by AiSidecarManager; this page only reacts to its state.
 */
class AIPage : public TempPage
{
    Q_OBJECT

public:
    explicit AIPage (QWidget *parent = nullptr);

    // Ensures the sidecar is running (or a start attempt is in flight) and
    // refreshes this page's state view. Called when entering AI mode.
    void activateAiMode ();

private slots:
    void onSidecarStateChanged (AiSidecarManager::State newState);
    void onSidecarReady (const QString &baseUrl);
    void onSidecarFailed (const QString &reason);

private:
    void initUI ();
    void onRetryClicked ();
    void onOpenBrowserClicked ();
    void showStatus (const QString &text, bool showRetry,
                     bool showOpenBrowser);
    void loadWebUi ();

    QWidget *m_statusWidget = nullptr;
    QLabel *m_statusLabel = nullptr;
    ElaPushButton *m_retryButton = nullptr;
    ElaPushButton *m_openBrowserButton = nullptr;

#ifdef TRANCE_AI_WEBENGINE
    QWebEngineView *m_webView = nullptr;
    bool m_webUiLoaded = false;
#endif
};
