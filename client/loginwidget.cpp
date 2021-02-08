#include "loginwidget.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QCoreApplication>

struct UiStatus {
    const char* text;
    const char* style;
};

UiStatus uiConnectionStatusList[]
{
    { "● Error",            "font-size: 16px; color: #FF0000" }, // ConnectionStatus::Error
    { "● Offline",          "font-size: 16px; color: #FF0000" }, // ConnectionStatus::Disconnected
    { "● Disconnecting...", "font-size: 16px; color: #FF0000" }, // ConnectionStatus::Disconnecting
    { "● Connecting..."   , "font-size: 16px; color: #FFA500" }, // ConnectionStatus::Connecting
    { "● Connected",        "font-size: 16px; color: #00FF00" }, // ConnectionStatus::Connected
};

UiStatus uiStatusList[]
{

    { "● Requesting...",     "font-size: 16px; color: #FFA500" }, // THORQ_STATE_CRYPTO_REQUESTED
    { "● Encrypting...",     "font-size: 16px; color: #FFA500" }, // THORQ_STATE_CRYPTO_ESTABLISHING
    { "● Verifying...",      "font-size: 16px; color: #FFA500" }, // THORQ_STATE_CRYPTO_VERIFYING
    { "● Encryped",          "font-size: 16px; color: #00FF00" }, // THORQ_STATE_AUTH_NONE

    { "● Authenticating...", "font-size: 16px; color: #FFA500" }, // THORQ_STATE_AUTH_HWID_CHECKING
    { "● Authenticated",     "font-size: 16px; color: #00FF00" }, // THORQ_STATE_AUTH_OK

    { "● Logging out...",    "font-size: 16px; color: #FFA500" }, // THORQ_STATE_LOGIN_LOGGINGOUT
    { "● Logging in...",     "font-size: 16px; color: #FFA500" }, // THORQ_STATE_LOGIN_LOGGINGIN
};

LoginWidget::LoginWidget(QWidget* parent)
	: QWidget(parent)
    , m_connectionStatus(ConnectionStatus::Error)
    , m_title(new QLabel(this))
    , m_onlineStatus(new QLabel(this))
    , m_textInput(new QLineEdit(this))
    , m_loginButton(new QPushButton(this))
    , m_forgotButton(new QPushButton(this))
    , m_registerButton(new QPushButton(this))
    , m_mainLayout(new QVBoxLayout(this))
    , m_headerLayout(new QHBoxLayout())
    , m_belowLoginLayout(new QHBoxLayout())
{
    setWindowTitle(tr("ThorQ Login"));

    m_title->setText("ThorQ");
    m_title->setStyleSheet("font-size: 72px; color: #FFFFFF");

    m_onlineStatus->setText(uiConnectionStatusList[(int)ConnectionStatus::Disconnected].text);
    m_onlineStatus->setStyleSheet(uiConnectionStatusList[(int)ConnectionStatus::Disconnected].style);

	m_headerLayout->addWidget(m_title);
    m_headerLayout->addWidget(m_onlineStatus);

	m_mainLayout->addLayout(m_headerLayout);
    m_mainLayout->addWidget(m_textInput);
    m_mainLayout->addWidget(m_loginButton);
    m_mainLayout->addLayout(m_belowLoginLayout);

    m_belowLoginLayout->addWidget(m_registerButton);
    m_belowLoginLayout->addWidget(m_forgotButton);

	setLayout(m_mainLayout);

	setFixedSize(m_mainLayout->geometry().size());
	setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);

    QObject::connect(m_loginButton, &QPushButton::clicked, [this]()
    {
        if (m_textInput->text().isEmpty())
            return;

        emit usernameEntered(m_textInput->text());
    });

    updateUiState();
}

LoginWidget::~LoginWidget()
{
	delete m_headerLayout;
}

void LoginWidget::setConnectionStatus(ConnectionStatus status)
{
    if (m_connectionStatus != status)
    {
        m_connectionStatus = status;
        m_onlineStatus->setText(uiConnectionStatusList[(int)status].text);
        m_onlineStatus->setStyleSheet(uiConnectionStatusList[(int)status].style);
        updateUiState();
    }
}

void LoginWidget::updateUiState()
{
    if (m_connectionStatus == ConnectionStatus::Connected)
    {
        m_textInput->setText("");
        m_textInput->show();

        m_loginButton->setText(tr("Login"));
        m_loginButton->show();

        m_registerButton->setText(tr("Register"));
        m_registerButton->show();

        m_forgotButton->setText(tr("Forgot"));
        m_forgotButton->show();

        adjustSize();
    }
    else
    {
        m_textInput->hide();
        m_loginButton->hide();
        m_registerButton->hide();
        m_forgotButton->hide();
        adjustSize();
    }
}
