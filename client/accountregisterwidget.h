#ifndef ACCOUNTREGISTERWIDGET_H
#define ACCOUNTREGISTERWIDGET_H

#include <QWidget>

#include "typedefs_client.h"

namespace ThorQ {
class AccountRegisterWidget : public QWidget
{
    Q_OBJECT
public:
    AccountRegisterWidget(ThorQ::AccountController* accountController, QWidget *parent = nullptr);
    ~AccountRegisterWidget();
signals:
    void goBackButtonPressed();
private:
    QPushButton* m_goBackButton;

    NamedLineEdit* m_usernameInput;
    NamedLineEdit* m_emailInput;
    NamedLineEdit* m_passwordInput;
    NamedLineEdit* m_confirmPasswordInput;
    QPushButton* m_registerButton;

    QVBoxLayout* m_layout;

    ThorQ::AccountController* m_accountController;
};
}

#endif // ACCOUNTREGISTERWIDGET_H
