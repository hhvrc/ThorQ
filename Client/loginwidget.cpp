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
{
	setWindowTitle("ThorQ Login");

	m_title = new QLabel("ThorQ", this);
	m_title->setStyleSheet("font-size: 72px");

	m_onlineStatus = new QLabel("● Offline", this);
	m_onlineStatus->setStyleSheet("font-size: 16px; color: #FF0000");

	m_headerLayout = new QHBoxLayout();
	m_headerLayout->addWidget(m_title);
	m_headerLayout->addWidget(m_onlineStatus);

	m_loginButton = new QPushButton("Login", this);
	m_usernameInput = new QLineEdit("Username", this);

	//auto shadow = new QGraphicsDropShadowEffect();
	//shadow->setXOffset(4);
	//shadow->setYOffset(4);
	//m_loginButton->setGraphicsEffect(shadow);

	m_mainLayout = new QVBoxLayout(this);
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

void LoginWidget::SetConnectionPing(int ping)
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
		m_onlineStatus->setStyleSheet("font-size: 16px; color: #FFA500");
		m_onlineStatus->setText(QString("● Connected"));
		m_usernameInput->show();
		m_loginButton->show();
		return;
	}

	m_usernameInput->hide();
	m_loginButton->hide();
}

void LoginWidget::updateUiLoginState()
{
	switch (m_loginState) {
	case THORQ_LOGIN_STATE_LOGGEDOUT:
		break;
	case THORQ_LOGIN_STATE_LOGGINGOUT:
		break;
	case THORQ_LOGIN_STATE_LOGGINGIN:
		break;
	case THORQ_LOGIN_STATE_LOGGEDIN:
		break;
	}
}

void LoginWidget::updateUiPing()
{
	if (m_connectionState != THORQ_CONNECTION_STATE_CONNECTED)
		return;

	m_onlineStatus->setText(QString("● Connected\n%1 ms").arg(m_ping));
}
