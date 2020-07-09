#include "loginwidget.h"

#include <QDebug>
#include <QLabel>
#include <QPalette>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

bool ddd = false;

LoginWidget::LoginWidget()
{
    m_vlayout = new QVBoxLayout(this);
    setLayout(m_vlayout);
    m_onlineStatus = new QLabel("● Offline", this);
    m_onlineStatus->setStyleSheet("QLabel { color: red; }");
    m_loginButton = new QPushButton("Login", this);

    setWindowTitle("ThorQ Login");

    m_vlayout->addWidget(m_onlineStatus);
    m_vlayout->addWidget(m_loginButton);

    connect(m_loginButton, &QPushButton::clicked, [this](bool checked)
    {
        m_onlineStatus->setStyleSheet(ddd ? "color: green" : "color: red");
        m_onlineStatus->setText(ddd ? "● Online" : "● Offline");
        ddd = !ddd;
    });
}
