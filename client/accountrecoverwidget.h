#ifndef ACCOUNTRECOVERWIDGET_H
#define ACCOUNTRECOVERWIDGET_H

#include <QWidget>

#include "typedefs_client.h"

namespace ThorQ {
class AccountRecoverWidget : public QWidget
{
     Q_OBJECT
public:
    AccountRecoverWidget(ThorQ::AccountController* accountController, QWidget *parent = nullptr);
    ~AccountRecoverWidget();
signals:
    void goBackButtonPressed();
private:
    QPushButton* m_goBackButton;

    NamedLineEdit* m_emailInput;
    QPushButton* m_recoverButton;

    QVBoxLayout* m_layout;

    ThorQ::AccountController* m_accountController;
};
}

#endif // ACCOUNTRECOVERWIDGET_H
