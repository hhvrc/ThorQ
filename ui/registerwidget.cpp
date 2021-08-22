#include "registerwidget.h"

#include "namedlineedit.h"
#include "validators.h"

#include <constants.h>

#include <QPushButton>
#include <QVBoxLayout>

ThorQ::UI::RegisterWidget::RegisterWidget(QWidget *parent)
    : QWidget(parent)
    , m_goBackButton(new QPushButton(this))
    , m_usernameInput(new NamedLineEdit(this))
    , m_emailInput(new NamedLineEdit(this))
    , m_passwordInput(new NamedLineEdit(this))
    , m_confirmPasswordInput(new NamedLineEdit(this))
    , m_registerButton(new QPushButton(this))
    , m_layout(new QVBoxLayout(this))
{
    // Go back button
    m_goBackButton->setText(tr("Go back"));
    m_goBackButton->setCursor(Qt::PointingHandCursor);
    QObject::connect(m_goBackButton, &QPushButton::clicked, this, &ThorQ::UI::RegisterWidget::handleGoBackClicked);

    // Register button
    m_registerButton->setText(tr("Register"));
    m_registerButton->setCursor(Qt::PointingHandCursor);
    QObject::connect(m_registerButton, &QPushButton::clicked, this, &ThorQ::UI::RegisterWidget::handleRegisterClicked);

    // Username input
    m_usernameInput->setName(tr("USERNAME"));
    m_usernameInput->setEchoMode(QLineEdit::EchoMode::Normal);
    m_usernameInput->setSimpleText(true);
    m_usernameInput->setInputValidator(ThorQ::Validators::UsernameValidator);
    QObject::connect(m_usernameInput, &ThorQ::UI::NamedLineEdit::textChanged, m_usernameInput, &ThorQ::UI::NamedLineEdit::hideError);
    QObject::connect(m_usernameInput, &ThorQ::UI::NamedLineEdit::textChanged, this, &ThorQ::UI::RegisterWidget::handleUsernameInputChanged);

    // Email input
    m_emailInput->setName(tr("EMAIL"));
    m_emailInput->setEchoMode(QLineEdit::EchoMode::Normal);
    m_emailInput->setSimpleText(true);
    m_emailInput->setInputValidator(ThorQ::Validators::EmailValidator);
    QObject::connect(m_emailInput, &ThorQ::UI::NamedLineEdit::textChanged, m_emailInput, &ThorQ::UI::NamedLineEdit::hideError);
    QObject::connect(m_emailInput, &ThorQ::UI::NamedLineEdit::textChanged, this, &ThorQ::UI::RegisterWidget::handleEmailInputChanged);

    // Password input
    m_passwordInput->setName(tr("PASSWORD"));
    m_passwordInput->setEchoMode(QLineEdit::EchoMode::Password);
    m_passwordInput->setInputValidator(ThorQ::Validators::EmailValidator);
    QObject::connect(m_passwordInput, &ThorQ::UI::NamedLineEdit::textChanged, m_passwordInput, &ThorQ::UI::NamedLineEdit::hideError);
    QObject::connect(m_passwordInput, &ThorQ::UI::NamedLineEdit::textChanged, this, &ThorQ::UI::RegisterWidget::handlePasswordInputChanged);

    // Confirm password input
    m_confirmPasswordInput->setName(tr("CONFIRM PASSWORD"));
    m_confirmPasswordInput->setEchoMode(QLineEdit::EchoMode::Password);
    QObject::connect(m_confirmPasswordInput, &ThorQ::UI::NamedLineEdit::textChanged, m_confirmPasswordInput, &ThorQ::UI::NamedLineEdit::hideError);
    QObject::connect(m_confirmPasswordInput, &ThorQ::UI::NamedLineEdit::textChanged, this, &ThorQ::UI::RegisterWidget::handlePasswordInputChanged);

    // Layout
    m_layout->setContentsMargins(12, 12, 12, 12);
    m_layout->addWidget(m_goBackButton);
    m_layout->addWidget(m_usernameInput);
    m_layout->addWidget(m_emailInput);
    m_layout->addWidget(m_passwordInput);
    m_layout->addWidget(m_confirmPasswordInput);
    m_layout->addWidget(m_registerButton);
    setLayout(m_layout);
}

ThorQ::UI::RegisterWidget::~RegisterWidget()
{
}

void ThorQ::UI::RegisterWidget::invalidateUsername(const QString& reason)
{
    m_usernameInput->showError(reason);
}

void ThorQ::UI::RegisterWidget::invalidateEmail(const QString& reason)
{
    m_emailInput->showError(reason);
}

void ThorQ::UI::RegisterWidget::invalidatePassword(const QString& reason)
{
    m_passwordInput->showError(reason);
}

void ThorQ::UI::RegisterWidget::clearForms()
{
    m_usernameInput->clearText();
    m_emailInput->clearText();
    clearPassword();
}

void ThorQ::UI::RegisterWidget::clearPassword()
{
    m_passwordInput->clearText();
    m_confirmPasswordInput->clearText();
}

void ThorQ::UI::RegisterWidget::handleGoBackClicked()
{
    emit goBackClicked();
}

void ThorQ::UI::RegisterWidget::handleRegisterClicked()
{
    emit registerClicked();
}

void ThorQ::UI::RegisterWidget::handleUsernameInputChanged()
{
    emit usernameChanged(m_usernameInput->text());
}

void ThorQ::UI::RegisterWidget::handleEmailInputChanged()
{
    emit emailChanged(m_emailInput->text());
}

void ThorQ::UI::RegisterWidget::handlePasswordInputChanged()
{
    QString password = m_passwordInput->text();
    if (password == m_confirmPasswordInput->text()) {
        emit passwordChanged(password);
    }
}
