#include "AiProviderDialog.h"

#include "Controller/AiProviderManager.h"
#include "Utility/Constants.h"
#include <ElaComboBox.h>
#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFont>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

AiProviderDialog::AiProviderDialog (QWidget *parent,
                                    const AiProviderConfig &existing)
    : QDialog (parent), m_existing (existing)
{
    setWindowTitle (existing.id >= 0 ? tr ("Edit AI Provider")
                                     : tr ("Add AI Provider"));
    setModal (true);
    setMinimumSize (520, 480);

    initUI ();
}

void AiProviderDialog::initUI ()
{
    QVBoxLayout *mainLayout = new QVBoxLayout (this);
    mainLayout->setSpacing (16);
    mainLayout->setContentsMargins (24, 24, 24, 24);

    QFormLayout *formLayout = new QFormLayout ();
    formLayout->setSpacing (12);
    formLayout->setLabelAlignment (Qt::AlignRight);

    const QFont labelFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                           Constants::Settings::DEFAULT_FONT_SIZE);

    auto makeLabel = [this, &labelFont] (const QString &text)
    {
        QLabel *label = new QLabel (text, this);
        label->setFont (labelFont);
        return label;
    };

    m_nameEdit = new ElaLineEdit (this);
    m_nameEdit->setPlaceholderText (tr ("e.g. OpenAI / DeepSeek / Local"));
    m_nameEdit->setText (m_existing.name);
    formLayout->addRow (makeLabel (tr ("Name")), m_nameEdit);

    m_baseUrlEdit = new ElaLineEdit (this);
    m_baseUrlEdit->setPlaceholderText (tr ("https://api.openai.com/v1"));
    m_baseUrlEdit->setText (m_existing.baseUrl);
    formLayout->addRow (makeLabel (tr ("Base URL")), m_baseUrlEdit);

    m_apiKeyEdit = new ElaLineEdit (this);
    m_apiKeyEdit->setPlaceholderText (
        m_existing.id >= 0 ? tr ("unchanged - leave empty to keep")
                           : tr ("sk-..."));
    m_apiKeyEdit->setEchoMode (QLineEdit::Password);
    formLayout->addRow (makeLabel (tr ("API Key")), m_apiKeyEdit);

    m_chatModelCombo = new ElaComboBox (this);
    m_chatModelCombo->setEditable (true);

    if (!m_existing.chatModel.isEmpty ())
    {
        m_chatModelCombo->addItem (m_existing.chatModel);
    }

    formLayout->addRow (makeLabel (tr ("Chat Model")), m_chatModelCombo);

    m_embeddingModelCombo = new ElaComboBox (this);
    m_embeddingModelCombo->setEditable (true);

    if (!m_existing.embeddingModel.isEmpty ())
    {
        m_embeddingModelCombo->addItem (m_existing.embeddingModel);
    }

    formLayout->addRow (makeLabel (tr ("Embedding Model (optional)")),
                        m_embeddingModelCombo);

    m_activeCheckBox =
        new QCheckBox (tr ("Use this provider (active)"), this);
    m_activeCheckBox->setChecked (m_existing.isActive
                                  || m_existing.id < 0);
    m_activeCheckBox->setFont (labelFont);

    m_testResultLabel = new QLabel (this);
    m_testResultLabel->setWordWrap (true);
    m_testResultLabel->setFont (QFont (
        Constants::Settings::DEFAULT_FONT_FAMILY,
        Constants::Settings::SMALL_FONT_SIZE));

    ElaPushButton *testButton =
        new ElaPushButton (tr ("Test Connection"), this);
    testButton->setMinimumHeight (40);

    QDialogButtonBox *buttonBox =
        new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                              this);

    mainLayout->addLayout (formLayout);
    mainLayout->addWidget (m_activeCheckBox);

    QHBoxLayout *testLayout = new QHBoxLayout ();
    testLayout->addWidget (testButton);
    testLayout->addWidget (m_testResultLabel, 1);
    mainLayout->addLayout (testLayout);
    mainLayout->addWidget (buttonBox);

    connect (testButton, &ElaPushButton::clicked, this,
             &AiProviderDialog::onTestConnectionClicked);

    connect (buttonBox, &QDialogButtonBox::accepted, this,
             &AiProviderDialog::onAcceptClicked);

    connect (buttonBox, &QDialogButtonBox::rejected, this,
             [this] () { reject (); });
}

void AiProviderDialog::onTestConnectionClicked ()
{
    m_testResultLabel->setText (tr ("Testing..."));
    m_testResultLabel->setStyleSheet ("color: #666;");

    AiProviderManager::getInstance ().testConnection (
        m_baseUrlEdit->text ().trimmed (), m_apiKeyEdit->text (),
        [this] (bool ok, const QString &errorMessage,
                const QStringList &models)
        {
            if (!ok)
            {
                m_testResultLabel->setText (
                    tr ("Failed: %1").arg (errorMessage));
                m_testResultLabel->setStyleSheet ("color: #F44336;");
                return;
            }

            m_testResultLabel->setText (
                tr ("OK - %1 models").arg (models.size ()));
            m_testResultLabel->setStyleSheet ("color: #4CAF50;");

            for (const QString &model : models)
            {
                if (m_chatModelCombo->findText (model) < 0)
                {
                    m_chatModelCombo->addItem (model);
                }

                if (m_embeddingModelCombo->findText (model) < 0)
                {
                    m_embeddingModelCombo->addItem (model);
                }
            }
        });
}

AiProviderConfig AiProviderDialog::config () const
{
    AiProviderConfig result = m_existing;
    result.name = m_nameEdit->text ().trimmed ();
    result.baseUrl = m_baseUrlEdit->text ().trimmed ();

    while (result.baseUrl.endsWith ("/"))
    {
        result.baseUrl.chop (1);
    }

    result.chatModel = m_chatModelCombo->currentText ().trimmed ();
    result.embeddingModel =
        m_embeddingModelCombo->currentText ().trimmed ();
    result.isActive = m_activeCheckBox->isChecked ();
    return result;
}

QString AiProviderDialog::apiKeyInput () const
{
    return m_apiKeyEdit->text ();
}

void AiProviderDialog::onAcceptClicked ()
{
    const AiProviderConfig entered = config ();

    if (entered.name.isEmpty () || entered.baseUrl.isEmpty ()
        || entered.chatModel.isEmpty ())
    {
        m_testResultLabel->setText (
            tr ("Name, Base URL and Chat Model are required."));
        m_testResultLabel->setStyleSheet ("color: #F44336;");
        return;
    }

    if (entered.baseUrl.isEmpty () || !entered.baseUrl.startsWith ("http"))
    {
        m_testResultLabel->setText (
            tr ("Base URL must start with http:// or https://"));
        m_testResultLabel->setStyleSheet ("color: #F44336;");
        return;
    }

    accept ();
}
