#include "loginwidget.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

const char* uiStatusList[THORQ_LOGIN_STATE_LOGGEDIN + 1][2]
{
    { "● Offline",                  "font-size: 16px; color: #FF0000" }, // THORQ_CONNECTION_STATE_DISCONNECTED
    { "● Disconnecting...",         "font-size: 16px; color: #FF0000" }, // THORQ_CONNECTION_STATE_DISCONNECTING
    { "● Connecting..."   ,         "font-size: 16px; color: #FFA500" }, // THORQ_CONNECTION_STATE_CONNECTING
    { "● Connected\n%1 ms",         "font-size: 16px; color: #00FF00" }, // THORQ_CONNECTION_STATE_CONNECTED

    { "● Requesting...\n%1 ms",     "font-size: 16px; color: #FFA500" }, // THORQ_CRYPTO_STATE_REQUESTED
    { "● Encrypting...\n%1 ms",     "font-size: 16px; color: #FFA500" }, // THORQ_CRYPTO_STATE_ESTABLISHING
    { "● Verifying...\n%1 ms",      "font-size: 16px; color: #FFA500" }, // THORQ_CRYPTO_STATE_VERIFYING
    { "● Encryped\n%1 ms",          "font-size: 16px; color: #00FF00" }, // THORQ_AUTH_STATE_NONE

    { "● Authenticating...\n%1 ms", "font-size: 16px; color: #FFA500" }, // THORQ_AUTH_STATE_HWID_CHECKING
    { "● Awaiting key...\n%1 ms",   "font-size: 16px; color: #FFA500" }, // THORQ_AUTH_STATE_REGKEY_AWAITING_INPUT
    { "● Registering...\n%1 ms",    "font-size: 16px; color: #FFA500" }, // THORQ_AUTH_STATE_REGKEY_CHECKING
    { "● Authenticated\n%1 ms",     "font-size: 16px; color: #00FF00" }, // THORQ_AUTH_STATE_OK

    { "● Logging out...\n%1 ms",    "font-size: 16px; color: #FFA500" }, // THORQ_LOGIN_STATE_LOGGINGOUT
    { "● Logging in...\n%1 ms",     "font-size: 16px; color: #FFA500" }, // THORQ_LOGIN_STATE_LOGGINGIN
    { "● Logged in\n%1 ms",         "font-size: 16px; color: #00FF00" }, // THORQ_LOGIN_STATE_LOGGEDIN
};

LoginWidget::LoginWidget(QWidget* parent)
	: QWidget(parent)
    , m_state(0)
	, m_ping(0)
    , m_title(new QLabel(this))
    , m_onlineStatus(new QLabel(this))
    , m_usernameInput(new QLineEdit(this))
    , m_loginButton(new QPushButton(this))
    , m_mainLayout(new QVBoxLayout(this))
    , m_headerLayout(new QHBoxLayout())
{
	setWindowTitle("ThorQ Login");

    m_title->setText("ThorQ");
	m_title->setStyleSheet("font-size: 72px");

    m_onlineStatus->setText("● Offline");
	m_onlineStatus->setStyleSheet("font-size: 16px; color: #FF0000");

    m_usernameInput->setText("Username");

    m_loginButton->setText("Login");

	m_headerLayout->addWidget(m_title);
    m_headerLayout->addWidget(m_onlineStatus);

	m_mainLayout->addLayout(m_headerLayout);
	m_mainLayout->addWidget(m_usernameInput);
	m_mainLayout->addWidget(m_loginButton);
	setLayout(m_mainLayout);

	setFixedSize(m_mainLayout->geometry().size());
	setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);

	connect(m_loginButton, &QPushButton::clicked, [this](){ emit LoginRequest(m_usernameInput->text()); });

    updateUiState();
}

void LoginWidget::SetConnectionState(thorq_connection_state_t state)
{
    if (m_state != state)
	{
        m_state = state;
        updateUiState();
    }
}

void LoginWidget::SetCryptoState(thorq_crypto_state_t state)
{
    if (m_state != state)
    {
        m_state = state;
        updateUiState();
    }
}

void LoginWidget::SetAuthState(thorq_auth_state_t state)
{
    if (m_state != state)
    {
        m_state = state;
        updateUiState();
    }
}

void LoginWidget::SetLoginState(thorq_login_state_t state)
{
    if (m_state != state)
    {
        m_state = state;
        updateUiState();
    }
}

void LoginWidget::SetConnectionPing(uint ping)
{
	if (m_ping != ping)
	{
		m_ping = ping;
		updateUiPing();
	}
}

void LoginWidget::updateUiState()
{
    m_onlineStatus->setStyleSheet(uiStatusList[m_state][1]);

    if (m_state < THORQ_CONNECTION_STATE_CONNECTED)
    {
        m_onlineStatus->setText(QString(uiStatusList[m_state][0]));
    }
    else
    {
        m_onlineStatus->setText(QString(uiStatusList[m_state][0]).arg(m_ping));
    }

    if (m_state == THORQ_LOGIN_STATE_LOGGEDOUT)
    {
        m_loginButton->show();
        m_usernameInput->show();
    }
    else
    {
        m_loginButton->hide();
        m_usernameInput->hide();
    }
}

void LoginWidget::updateUiPing()
{
    if (m_state >= THORQ_CONNECTION_STATE_CONNECTED)
    {
        m_onlineStatus->setText(QString(uiStatusList[m_state][0]).arg(m_ping));
    }
}
