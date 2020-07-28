#include "loginwidget.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

quint64 ddd = 0;

LoginWidget::LoginWidget(QWidget* parent)
	: QWidget(parent)
	, m_connectionState(THORQ_CONNECTION_STATE_DISCONNECTED)
	, m_loginState(THORQ_LOGIN_STATE_LOGGEDOUT)
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

	updateUiConnectionState();
}

LoginWidget::~LoginWidget()
{

}

void LoginWidget::SetConnectionState(thorq_connection_state_t state)
{
	if (m_connectionState != state)
	{
		m_connectionState = state;
		updateUiConnectionState();
	}
}

void LoginWidget::SetLoginState(thorq_login_state_t state)
{
	if (m_loginState != state)
	{
		m_loginState = state;
		updateUiLoginState();
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

void LoginWidget::updateUiConnectionState()
{
	switch (m_connectionState) {
	case THORQ_CONNECTION_STATE_DISCONNECTED:
		m_onlineStatus->setStyleSheet("font-size: 16px; color: #FF0000");
		m_onlineStatus->setText(QString("● Offline"));
		break;
	case THORQ_CONNECTION_STATE_DISCONNECTING:
		m_onlineStatus->setStyleSheet("font-size: 16px; color: #FF0000");
		m_onlineStatus->setText(QString("● Disconnecting..."));
		break;
	case THORQ_CONNECTION_STATE_CONNECTING:
		m_onlineStatus->setStyleSheet("font-size: 16px; color: #FFA500");
		m_onlineStatus->setText(QString("● Connecting..."));
		break;
	case THORQ_CONNECTION_STATE_CONNECTED:
        m_onlineStatus->setStyleSheet("font-size: 16px; color: #00FF00");
		m_onlineStatus->setText(QString("● Connected"));
        m_loginButton->show();
        m_usernameInput->show();
		return;
	}

    m_loginButton->hide();
    m_usernameInput->hide();
}

void LoginWidget::updateUiLoginState()
{
	switch (m_loginState) {
	case THORQ_LOGIN_STATE_LOGGEDOUT:
        m_usernameInput->setEnabled(true);
        m_loginButton->setEnabled(true);
        setCursor(Qt::ArrowCursor);
        setVisible(true);
        return;
	case THORQ_LOGIN_STATE_LOGGINGOUT:
        setCursor(Qt::WaitCursor);
		break;
	case THORQ_LOGIN_STATE_LOGGINGIN:
        setCursor(Qt::WaitCursor);
		break;
	case THORQ_LOGIN_STATE_LOGGEDIN:
        setCursor(Qt::ArrowCursor);
        setVisible(false);
		break;
	}

    m_usernameInput->setEnabled(false);
    m_loginButton->setEnabled(false);
}

void LoginWidget::updateUiPing()
{
	if (m_connectionState != THORQ_CONNECTION_STATE_CONNECTED)
		return;

	m_onlineStatus->setText(QString("● Connected\n%1 ms").arg(m_ping));
}
