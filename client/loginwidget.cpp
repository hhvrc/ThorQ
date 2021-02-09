#include "loginwidget.h"

#include "accountcontroller.h"
#include "namedlineedit.h"

#include <constants.h>

#include <QDebug>
#include <QLabel>
#include <QStyle>
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

ThorQ::LoginWidget::LoginWidget(ThorQ::AccountController* accountController, QWidget* parent)
	: QWidget(parent)
    , m_connectionStatus(ConnectionStatus::Error)
    , m_title(new QLabel(this))
    , m_onlineStatus(new QLabel(this))
    , m_usernameInput(new NamedLineEdit(this))
    , m_passwordInput(new NamedLineEdit(this))
    , m_loginButton(new QPushButton(this))
    , m_forgotButton(new QPushButton(this))
    , m_registerButton(new QPushButton(this))
    , m_mainLayout(new QVBoxLayout(this))
    , m_headerLayout(new QHBoxLayout())
    , m_belowLoginLayout(new QHBoxLayout())
    , m_accountController(accountController)
{
    setWindowTitle(tr("ThorQ Login"));

    m_title->setText("ThorQ");
    m_title->setStyleSheet("font-size: 72px; color: #FFFFFF");

    m_onlineStatus->setText(uiConnectionStatusList[(int)ConnectionStatus::Disconnected].text);
    m_onlineStatus->setStyleSheet(uiConnectionStatusList[(int)ConnectionStatus::Disconnected].style);

    m_usernameInput->setName(tr("USERNAME"));
    m_usernameInput->setEchoMode(QLineEdit::EchoMode::Normal);
    m_passwordInput->setName(tr("PASSWORD"));
    m_passwordInput->setEchoMode(QLineEdit::EchoMode::Password);

    m_loginButton->setText(tr("Login"));
    m_registerButton->setText(tr("Register"));
    m_forgotButton->setText(tr("Forgot"));

    m_headerLayout->setContentsMargins(0, 0, 0, 0);
	m_headerLayout->addWidget(m_title);
    m_headerLayout->addWidget(m_onlineStatus);

    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->addLayout(m_headerLayout);
    m_mainLayout->addWidget(m_usernameInput);
    m_mainLayout->addWidget(m_passwordInput);
    m_mainLayout->addWidget(m_loginButton);
    m_mainLayout->addLayout(m_belowLoginLayout);

    m_belowLoginLayout->setContentsMargins(0, 0, 0, 0);
    m_belowLoginLayout->addWidget(m_registerButton);
    m_belowLoginLayout->addWidget(m_forgotButton);

	setLayout(m_mainLayout);

    setFixedSize(m_mainLayout->geometry().size());
    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);

    QObject::connect(m_loginButton, &QPushButton::clicked, [this]() {
        if (m_accountController != nullptr) {
            auto username = m_usernameInput->text();
            auto password = m_passwordInput->text();

            if (username.size() < THORQ_USERNAME_LEN_MIN || username.size() > THORQ_USERNAME_LEN_MAX ||
                password.size() < THORQ_PASSWORD_LEN_MIN || password.size() > THORQ_PASSWORD_LEN_MAX) {
                return;
            }

            m_passwordInput->setText("");

            m_accountController->setUsername(username);
            m_accountController->setPassword(password);
            m_accountController->login();
        }
    });

    updateUiState();
}

ThorQ::LoginWidget::~LoginWidget()
{
	delete m_headerLayout;
}

void ThorQ::LoginWidget::setConnectionStatus(ConnectionStatus status)
{
    if (m_connectionStatus != status)
    {
        m_connectionStatus = status;
        m_onlineStatus->setText(uiConnectionStatusList[(int)status].text);
        m_onlineStatus->setStyleSheet(uiConnectionStatusList[(int)status].style);
        updateUiState();
    }
}

void ThorQ::LoginWidget::updateUiState()
{
    if (m_connectionStatus == ConnectionStatus::Connected)
    {
        m_usernameInput->show();

        m_passwordInput->setText("");
        m_passwordInput->show();

        m_loginButton->show();
        m_registerButton->show();
        m_forgotButton->show();

        adjustSize();
    }
    else
    {
        m_usernameInput->hide();
        m_passwordInput->hide();
        m_loginButton->hide();
        m_registerButton->hide();
        m_forgotButton->hide();
        adjustSize();
    }
}
