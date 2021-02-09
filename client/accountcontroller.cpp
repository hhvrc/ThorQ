#include "accountcontroller.h"

#include <QDebug>

ThorQ::AccountController::AccountController(QObject *parent)
    : QObject(parent)
    , m_username()
    , m_password()
{
}

void ThorQ::AccountController::setUsername(const QString& username)
{
    qDebug() << username;
    m_username = username;
}

void ThorQ::AccountController::setPassword(const QString& password)
{
    qDebug() << password;
    m_password = password;
}

void ThorQ::AccountController::login()
{
    qDebug() << "Login";
}

void ThorQ::AccountController::logout()
{
    qDebug() << "Logout";
}
