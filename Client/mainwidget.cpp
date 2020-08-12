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

void MainWidget::setLoginState(THORQ_STATE_LOGIN state)
{
	if (m_state != state)
	{
		m_state = state;
		updateUiState();
	}
}

void MainWidget::setSessionState(THORQ_STATE_SESSION state)
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
	if (m_state < THORQ_STATE_LOGIN_LOGGEDIN)
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
