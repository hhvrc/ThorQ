#include "loginwidget.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

const char* uiStatusList[][2]
{
    { "● Offline",                  "font-size: 16px; color: #FF0000" }, // THORQ_STATE_CONNECTION_DISCONNECTED
    { "● Disconnecting...",         "font-size: 16px; color: #FF0000" }, // THORQ_STATE_CONNECTION_DISCONNECTING
    { "● Connecting..."   ,         "font-size: 16px; color: #FFA500" }, // THORQ_STATE_CONNECTION_CONNECTING
    { "● Connected\n%1 ms",         "font-size: 16px; color: #00FF00" }, // THORQ_STATE_CONNECTION_CONNECTED

    { "● Requesting...\n%1 ms",     "font-size: 16px; color: #FFA500" }, // THORQ_STATE_CRYPTO_REQUESTED
    { "● Encrypting...\n%1 ms",     "font-size: 16px; color: #FFA500" }, // THORQ_STATE_CRYPTO_ESTABLISHING
    { "● Verifying...\n%1 ms",      "font-size: 16px; color: #FFA500" }, // THORQ_STATE_CRYPTO_VERIFYING
    { "● Encryped\n%1 ms",          "font-size: 16px; color: #00FF00" }, // THORQ_STATE_AUTH_NONE

    { "● Authenticating...\n%1 ms", "font-size: 16px; color: #FFA500" }, // THORQ_STATE_AUTH_HWID_CHECKING
    { "● Awaiting key...\n%1 ms",   "font-size: 16px; color: #FFA500" }, // THORQ_STATE_AUTH_REGKEY_AWAITING_INPUT
    { "● Registering...\n%1 ms",    "font-size: 16px; color: #FFA500" }, // THORQ_STATE_AUTH_REGKEY_CHECKING
    { "● Authenticated\n%1 ms",     "font-size: 16px; color: #00FF00" }, // THORQ_STATE_AUTH_OK

    { "● Logging out...\n%1 ms",    "font-size: 16px; color: #FFA500" }, // THORQ_STATE_LOGIN_LOGGINGOUT
    { "● Logging in...\n%1 ms",     "font-size: 16px; color: #FFA500" }, // THORQ_STATE_LOGIN_LOGGINGIN
    { "● Logged in\n%1 ms",         "font-size: 16px; color: #00FF00" }, // THORQ_STATE_LOGIN_LOGGEDIN
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
    setWindowTitle(tr("ThorQ Login"));

    m_title->setText("ThorQ");
    m_title->setStyleSheet("font-size: 72px; color: #FFFFFF");

    m_onlineStatus->setText(tr(uiStatusList[0][0]));
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
        if (m_textInput->text().isEmpty())
            return;

        if (m_state == THORQ_STATE_AUTH_REGKEY_AWAITING_INPUT)
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

LoginWidget::~LoginWidget()
{
	delete m_headerLayout;
}

void LoginWidget::setConnectionState(THORQ_STATE_CONNECTION state)
{
    if (m_state != state)
	{
        m_state = state;
        updateUiState();
    }
}

void LoginWidget::setCryptoState(THORQ_STATE_CRYPTO state)
{
    if (m_state != state)
    {
        m_state = state;
        updateUiState();
    }
}

void LoginWidget::setAuthState(THORQ_STATE_AUTH state)
{
    if (m_state != state)
    {
        m_state = state;
        updateUiState();
    }
}

void LoginWidget::setLoginState(THORQ_STATE_LOGIN state)
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

    if (m_state < THORQ_STATE_CONNECTION_CONNECTED)
    {
        m_onlineStatus->setText(tr(uiStatusList[m_state][0]));
    }
    else
    {
        m_onlineStatus->setText(tr(uiStatusList[m_state][0]).arg(m_ping));
    }

    if (m_state == THORQ_STATE_AUTH_REGKEY_AWAITING_INPUT)
    {
        m_textInput->setText("");
        m_textInput->show();

        m_acceptButton->setText(tr("Submit"));
        m_acceptButton->show();

        adjustSize();
    }
    else if (m_state == THORQ_STATE_LOGIN_LOGGEDOUT)
    {
        m_textInput->setText("");
        m_textInput->show();

        m_acceptButton->setText(tr("Login"));
        m_acceptButton->show();

        adjustSize();
    }
    else
    {
        m_acceptButton->hide();
        m_textInput->hide();
        adjustSize();
    }

	if (m_state < THORQ_STATE_LOGIN_LOGGEDIN)
	{
		show();
	}
	else
	{
		hide();
	}
}

void LoginWidget::updateUiPing()
{
    if (m_state >= THORQ_STATE_CONNECTION_CONNECTED)
    {
        m_onlineStatus->setText(tr(uiStatusList[m_state][0]).arg(m_ping));
    }
}
