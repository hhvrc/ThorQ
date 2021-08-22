#include "loginwidget.h"

#include "namedlineedit.h"
#include "constants.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "validators.h"

ThorQ::UI::LoginWidget::LoginWidget(QWidget* parent)
    : QWidget(parent)
    , m_title(new QLabel(this))
    , m_onlineStatus(new QLabel(this))
    , m_usernameInput(new NamedLineEdit(this))
    , m_passwordInput(new NamedLineEdit(this))
    , m_loginButton(new QPushButton(this))
    , m_mainLayout(new QVBoxLayout(this))
    , m_headerLayout(new QHBoxLayout())
{
    m_title->setText(ThorQ::APPLICATION_NAME);
    m_title->setStyleSheet("font-size: 72px; color: #FFFFFF");

    m_usernameInput->setName(tr("USERNAME"));
    m_usernameInput->setEchoMode(QLineEdit::EchoMode::Normal);
    m_usernameInput->setSimpleText(true);
    m_usernameInput->setInputValidator(ThorQ::Validators::UsernameValidator);

    m_passwordInput->setName(tr("PASSWORD"));
    m_passwordInput->setEchoMode(QLineEdit::EchoMode::Password);
    m_passwordInput->setInputValidator(ThorQ::Validators::PasswordValidator);

    m_loginButton->setText(tr("Login"));
    m_loginButton->setCursor(Qt::PointingHandCursor);

    m_headerLayout->setAlignment(Qt::AlignTop);
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    m_headerLayout->addWidget(m_title);
    m_headerLayout->addWidget(m_onlineStatus);

    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->addLayout(m_headerLayout, 1);
    m_mainLayout->addWidget(m_usernameInput);
    m_mainLayout->addWidget(m_passwordInput);
    m_mainLayout->addWidget(m_loginButton);

    setLayout(m_mainLayout);

    QObject::connect(m_usernameInput, &ThorQ::UI::NamedLineEdit::textChanged, this, &ThorQ::UI::LoginWidget::handleUsernameInputChanged);
    QObject::connect(m_passwordInput, &ThorQ::UI::NamedLineEdit::textChanged, this, &ThorQ::UI::LoginWidget::handlePasswordInputChanged);
    QObject::connect(m_loginButton, &QPushButton::clicked, this, &ThorQ::UI::LoginWidget::handleLoginClicked);
}

ThorQ::UI::LoginWidget::~LoginWidget()
{
    delete m_headerLayout;
}

void ThorQ::UI::LoginWidget::setOnlineStatus(const QString& status, QColor color)
{
    m_onlineStatus->setText(QString("● %1").arg(status));
    m_onlineStatus->setStyleSheet(QString("font-size: 16px; color: %1").arg(color.name()));
}

void ThorQ::UI::LoginWidget::invalidateUsername(const QString& reason)
{
    m_usernameInput->showError(reason);
}

void ThorQ::UI::LoginWidget::invalidatePassword(const QString& reason)
{
    m_passwordInput->showError(reason);
}

void ThorQ::UI::LoginWidget::clearForms()
{
    m_usernameInput->clearText();
    m_passwordInput->clearText();
    clearPassword();
}

void ThorQ::UI::LoginWidget::clearPassword()
{
    m_passwordInput->clearText();
}

void ThorQ::UI::LoginWidget::handleUsernameInputChanged()
{
    m_usernameInput->hideError();
    emit usernameChanged(m_usernameInput->text());
}

void ThorQ::UI::LoginWidget::handlePasswordInputChanged()
{
    m_passwordInput->hideError();
    emit passwordChanged(m_passwordInput->text());
}

void ThorQ::UI::LoginWidget::handleLoginClicked()
{
    emit loginClicked();
}
