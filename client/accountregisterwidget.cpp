#include "accountregisterwidget.h"

#include "accountcontroller.h"
#include "namedlineedit.h"

#include <constants.h>

#include <QPushButton>
#include <QVBoxLayout>

ThorQ::AccountRegisterWidget::AccountRegisterWidget(ThorQ::AccountController* accountController, QWidget *parent)
    : QWidget(parent)
    , m_goBackButton(new QPushButton(this))
    , m_usernameInput(new NamedLineEdit(this))
    , m_emailInput(new NamedLineEdit(this))
    , m_passwordInput(new NamedLineEdit(this))
    , m_confirmPasswordInput(new NamedLineEdit(this))
    , m_registerButton(new QPushButton(this))
    , m_layout(new QVBoxLayout(this))
    , m_accountController(accountController)
{
    m_goBackButton->setText(tr("Go back"));
    m_goBackButton->setCursor(Qt::PointingHandCursor);

    m_usernameInput->setName(tr("USERNAME"));
    m_usernameInput->setEchoMode(QLineEdit::EchoMode::Normal);

    m_emailInput->setName(tr("EMAIL"));
    m_emailInput->setEchoMode(QLineEdit::EchoMode::Normal);

    m_passwordInput->setName(tr("PASSWORD"));
    m_passwordInput->setEchoMode(QLineEdit::EchoMode::Password);

    m_confirmPasswordInput->setName(tr("CONFIRM PASSWORD"));
    m_confirmPasswordInput->setEchoMode(QLineEdit::EchoMode::Password);

    m_registerButton->setText(tr("Register"));
    m_registerButton->setCursor(Qt::PointingHandCursor);

    m_layout->setContentsMargins(12, 12, 12, 12);
    m_layout->addWidget(m_goBackButton);
    m_layout->addWidget(m_usernameInput);
    m_layout->addWidget(m_emailInput);
    m_layout->addWidget(m_passwordInput);
    m_layout->addWidget(m_confirmPasswordInput);
    m_layout->addWidget(m_registerButton);

    setLayout(m_layout);

    QObject::connect(m_goBackButton, &QPushButton::clicked, this, &QWidget::hide);
    QObject::connect(m_goBackButton, &QPushButton::clicked, [this](){ emit goBackButtonPressed(); });
    QObject::connect(m_registerButton, &QPushButton::clicked, [this]() {
        if (m_accountController != nullptr) {
            auto username = m_usernameInput->text();
            auto email = m_emailInput->text();
            auto password = m_passwordInput->text();

            if (username.size() < THORQ_USERNAME_LEN_MIN || username.size() > THORQ_USERNAME_LEN_MAX ||
                password.size() < THORQ_PASSWORD_LEN_MIN || password.size() > THORQ_PASSWORD_LEN_MAX ||
                email.size()    < THORQ_EMAIL_LEN_MIN    || email.size()    > THORQ_EMAIL_LEN_MAX    ||
                password != m_confirmPasswordInput->text()) {
                return;
            }

            m_passwordInput->setText("");
            m_confirmPasswordInput->setText("");
            m_accountController->registerAccount(username, email, password);
        }
    });
}

ThorQ::AccountRegisterWidget::~AccountRegisterWidget()
{

}
