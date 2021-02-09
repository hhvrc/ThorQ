#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

#include "enums.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class NamedLineEdit;
class ClickableLabel;

namespace ThorQ {
class SecureString;
class AccountController;
class LoginWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(LoginWidget)
public:
    LoginWidget(ThorQ::AccountController* accountController, QWidget* parent = nullptr);
    ~LoginWidget();
public slots:
    void setConnectionStatus(ConnectionStatus status);
private slots:
    void updateUiState();
private:
    ConnectionStatus m_connectionStatus;

    QLabel* m_title;
    QLabel* m_onlineStatus;
    NamedLineEdit* m_usernameInput;
    NamedLineEdit* m_passwordInput;
    QPushButton* m_loginButton;
    QPushButton* m_forgotButton;
    QPushButton* m_registerButton;

    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_headerLayout;
    QHBoxLayout* m_belowLoginLayout;

    ThorQ::AccountController* m_accountController;
};
}

#endif // LOGINWIDGET_H
