#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

#include <enums.h>

#include "typedefs_client.h"

namespace ThorQ {
class AccountLoginWidget : public QWidget
{
    Q_OBJECT
public:
    AccountLoginWidget(ThorQ::AccountController* accountController, QWidget* parent = nullptr);
    ~AccountLoginWidget();
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

    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_headerLayout;

    ThorQ::AccountController* m_accountController;
};
}

#endif // LOGINWIDGET_H
