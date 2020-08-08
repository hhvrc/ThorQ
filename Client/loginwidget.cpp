#include "loginwidget.h"

#include <QDebug>
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
    , m_textInput(new QLineEdit(this))
    , m_acceptButton(new QPushButton(this))
    , m_mainLayout(new QVBoxLayout(this))
    , m_headerLayout(new QHBoxLayout())
{
	setWindowTitle("ThorQ Login");

    m_title->setText("ThorQ");
	m_title->setStyleSheet("font-size: 72px");

    m_onlineStatus->setText("● Offline");
    m_onlineStatus->setStyleSheet("font-size: 16px; color: #FF0000");

	m_headerLayout->addWidget(m_title);
    m_headerLayout->addWidget(m_onlineStatus);

	m_mainLayout->addLayout(m_headerLayout);
    m_mainLayout->addWidget(m_textInput);
    m_mainLayout->addWidget(m_acceptButton);
	setLayout(m_mainLayout);

	setFixedSize(m_mainLayout->geometry().size());
	setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);

    QObject::connect(m_acceptButton, &QPushButton::clicked, [this]()
    {
        if (m_state == THORQ_AUTH_STATE_REGKEY_AWAITING_INPUT)
        {
            emit regkeyEntered(m_textInput->text());
        }
        else
        {
            emit usernameEntered(m_textInput->text());
        }
    });

    updateUiState();
}

void LoginWidget::setConnectionState(thorq_connection_state_t state)
{
    if (m_state != state)
	{
        m_state = state;
        updateUiState();
    }
}

void LoginWidget::setCryptoState(thorq_crypto_state_t state)
{
    if (m_state != state)
    {
        m_state = state;
        updateUiState();
    }
}

void LoginWidget::setAuthState(thorq_auth_state_t state)
{
    if (m_state != state)
    {
        m_state = state;
        updateUiState();
    }
}

void LoginWidget::setLoginState(thorq_login_state_t state)
{
    if (m_state != state)
    {
        m_state = state;
        updateUiState();
    }
}

void LoginWidget::setConnectionPing(uint ping)
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

    if (m_state == THORQ_AUTH_STATE_REGKEY_AWAITING_INPUT)
    {
        m_textInput->setText("");
        m_textInput->show();

        m_acceptButton->setText("Submit");
        m_acceptButton->show();
    }
    else if (m_state == THORQ_LOGIN_STATE_LOGGEDOUT)
    {
        m_textInput->setText("");
        m_textInput->show();

        m_acceptButton->setText("Login");
        m_acceptButton->show();
    }
    else
    {
        m_acceptButton->hide();
        m_textInput->hide();
    }
}

void LoginWidget::updateUiPing()
{
    if (m_state >= THORQ_CONNECTION_STATE_CONNECTED)
    {
        m_onlineStatus->setText(QString(uiStatusList[m_state][0]).arg(m_ping));
    }
}
