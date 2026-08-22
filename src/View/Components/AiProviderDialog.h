#pragma once

#include "Utility/AiProviderConfig.h"
#include <QDialog>

class ElaComboBox;
class ElaLineEdit;
class QLabel;
class QCheckBox;

/**
 * @brief Modal dialog for creating or editing an OpenAI-compatible AI
 *        provider entry (name / base URL / API key / models). Includes a
 *        "test connection" action that queries {baseUrl}/models and fills
 *        the model pickers.
 */
class AiProviderDialog : public QDialog
{
    Q_OBJECT

public:
    // Pass an existing config to edit it; a default-constructed config
    // starts a new entry.
    explicit AiProviderDialog (QWidget *parent,
                               const AiProviderConfig &existing = {});

    // Config as entered; id/name/baseUrl/... ready to be persisted via
    // AiProviderManager::saveProvider together with apiKeyInput().
    AiProviderConfig config () const;

    // Plain API key as typed; empty when the user did not (re)enter one,
    // in which case the previously stored key is kept on update.
    QString apiKeyInput () const;

private slots:
    void onTestConnectionClicked ();
    void onAcceptClicked ();

private:
    void initUI ();

    AiProviderConfig m_existing;

    ElaLineEdit *m_nameEdit = nullptr;
    ElaLineEdit *m_baseUrlEdit = nullptr;
    ElaLineEdit *m_apiKeyEdit = nullptr;
    ElaComboBox *m_chatModelCombo = nullptr;
    ElaComboBox *m_embeddingModelCombo = nullptr;
    QCheckBox *m_activeCheckBox = nullptr;
    QLabel *m_testResultLabel = nullptr;
};
