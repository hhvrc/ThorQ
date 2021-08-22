#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>

class QPushButton;
class QVBoxLayout;

namespace ThorQ::UI {

class NamedLineEdit;

class RegisterWidget : public QWidget
{
    Q_OBJECT
public:
    RegisterWidget(QWidget *parent = nullptr);
    ~RegisterWidget();
signals:
    void usernameChanged(const QString& email);
    void emailChanged(const QString& email);
    void passwordChanged(const QString& email);
    void goBackClicked();
    void registerClicked();
public slots:
    void invalidateUsername(const QString& reason);
    void invalidateEmail(const QString& reason);
    void invalidatePassword(const QString& reason);
    void clearForms();
    void clearPassword();
private slots:
    void handleGoBackClicked();
    void handleRegisterClicked();
    void handleUsernameInputChanged();
    void handleEmailInputChanged();
    void handlePasswordInputChanged();
private:
    QPushButton* m_goBackButton;

    NamedLineEdit* m_usernameInput;
    NamedLineEdit* m_emailInput;
    NamedLineEdit* m_passwordInput;
    NamedLineEdit* m_confirmPasswordInput;
    QPushButton* m_registerButton;

    QVBoxLayout* m_layout;
};

}

#endif // REGISTERWIDGET_H
