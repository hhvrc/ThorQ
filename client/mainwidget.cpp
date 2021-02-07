#include "mainwidget.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QListView>

#include "userlistitem.h"
#include "userlistmodel.h"

MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent)
    , m_logoutButton(new QPushButton(this))
	, m_vlayout(new QVBoxLayout(this))
	, m_hlayout(new QHBoxLayout())
{
	m_logoutButton->setText("Logout");
    QObject::connect(m_logoutButton, &QPushButton::clicked, [this](){ emit logoutButtonClicked(); });

    m_vlayout->addWidget(m_logoutButton);
	setLayout(m_vlayout);
}

void MainWidget::setConnectionPing(uint ping)
{
	if (m_ping != ping)
	{
		m_ping = ping;
		updateUiPing();
    }
}

void MainWidget::updateUser(const QString& username, uint8_t state)
{
   // m_userList->addItem(username);
}

void MainWidget::removeUser(const QString& username)
{
    //m_userModel->removeUser(username);
}

void MainWidget::clearUsers()
{
   // m_userList->clear();
    //m_userModel->clearUsers();
}

void MainWidget::updateUiState()
{
}

void MainWidget::updateUiPing()
{

}
