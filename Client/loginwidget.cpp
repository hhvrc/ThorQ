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
    , m_state(ThorQ::ClientState::Disconnected)
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

    updateStatus();
}

LoginWidget::~LoginWidget()
{

}

void LoginWidget::SetState(qint16 state)
{
    if (m_state != state)
    {
        m_state = state;
        updateStatus();
    }
}

void LoginWidget::SetConnectionPing(int ping)
{
    if (m_ping != ping)
    {
        m_ping = ping;

        if (m_state > ThorQ::ClientState::Connecting)
            updateStatus();
    }
}

void LoginWidget::updateStatus()
{
    switch (m_state) {
    case ThorQ::ClientState::Disconnected:
        m_onlineStatus->setStyleSheet("font-size: 16px; color: #FF0000");
        m_onlineStatus->setText(QString("● Offline"));
        break;
    case ThorQ::ClientState::Disconnecting:
        m_onlineStatus->setStyleSheet("font-size: 16px; color: #FF0000");
        m_onlineStatus->setText(QString("● Disconnecting..."));
        break;
    case ThorQ::ClientState::Connecting:
        m_onlineStatus->setStyleSheet("font-size: 16px; color: #FFA500");
        m_onlineStatus->setText(QString("● Connecting..."));
        break;
    case ThorQ::ClientState::Connected:
        m_onlineStatus->setStyleSheet("font-size: 16px; color: #FFA500");
        m_onlineStatus->setText(QString("● Connected\n%1 ms").arg(m_ping));
        m_usernameInput->show();
        m_loginButton->show();
        return;
    default:
        m_onlineStatus->setStyleSheet("font-size: 16px; color: #FFFFFF");
        m_onlineStatus->setText(QString("● ????"));
        break;
    }

    m_usernameInput->hide();
    m_loginButton->hide();
}
