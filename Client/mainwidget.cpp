#include "mainwidget.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QListWidget>

#include "usermodel.h"
#include "userdelegate.h"

MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent)
	, m_logoutButton(new QPushButton(this))
	, m_listView(new QListView(this))
	, m_userModel(new UserModel(this))
	, m_userDelegate(new UserDelegate(this))
	, m_vlayout(new QVBoxLayout(this))
	, m_hlayout(new QHBoxLayout())
{
	m_logoutButton->setText("Logout");
	QObject::connect(m_logoutButton, &QPushButton::clicked, [this](){ emit logoutButtonClicked(); });

	m_listView->setModel(m_userModel);

	m_vlayout->addWidget(m_logoutButton);
	m_vlayout->addWidget(m_listView);

	setLayout(m_vlayout);
}

void MainWidget::setLoginState(THORQ_STATE_LOGIN newState)
{
	if (newState < THORQ_STATE_LOGIN_LOGGEDIN)
	{
		hide();
	}
	else
	{
		show();
	}
}
void MainWidget::setSessionState(THORQ_STATE_SESSION newState)
{
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
}

void MainWidget::updateUiPing()
{

}
