#include "mainwidget.h"

#include <QBoxLayout>
#include <QPushButton>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
	, m_logoutButton(new QPushButton(this))
	, m_vlayout(new QVBoxLayout(this))
	, m_hlayout(new QHBoxLayout())
{
	QObject::connect(m_logoutButton, &QPushButton::clicked, [this](){ emit logoutButtonClicked(); });
	m_vlayout->addWidget(m_logoutButton);
	setLayout(m_vlayout);
}

void MainWidget::setLoginState(thorq_login_state_t state)
{
	if (m_state != state)
	{
		m_state = state;
		updateUiState();
	}
}

void MainWidget::setSessionState(thorq_session_state_t state)
{
	if (m_state != state)
	{
		m_state = state;
		updateUiState();
	}
}

void MainWidget::setConnectionPing(uint ping)
{
	if (m_ping != ping)
	{
		m_ping = ping;
		updateUiPing();
	}
}

void MainWidget::updateUiState()
{
	if (m_state < THORQ_LOGIN_STATE_LOGGEDIN)
	{
		hide();
	}
	else
	{
		show();
	}
}

void MainWidget::updateUiPing()
{

}
