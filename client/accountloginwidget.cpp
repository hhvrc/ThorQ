#include "accountloginwidget.h"

#include <QDebug>
#include <QLabel>
#include <QStyle>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QCoreApplication>

#include <constants.h>

#include "accountcontroller.h"
#include "namedlineedit.h"

struct LabelParams {
    const char* text;
    const char* style;
};
constexpr LabelParams ConnectionStatusLabelParams[]
{
    { "● Error",            "font-size: 16px; color: #FF0000" }, // ConnectionStatus::Error
    { "● Offline",          "font-size: 16px; color: #FF0000" }, // ConnectionStatus::Disconnected
    { "● Disconnecting...", "font-size: 16px; color: #FF0000" }, // ConnectionStatus::Disconnecting
    { "● Connecting..."   , "font-size: 16px; color: #FFA500" }, // ConnectionStatus::Connecting
    { "● Connected",        "font-size: 16px; color: #00FF00" }, // ConnectionStatus::Connected
};
constexpr const char* getConnectionStatusLabelText(ConnectionStatus status)
{
    return ConnectionStatusLabelParams[(int)status].text;
}
constexpr const char* getConnectionStatusLabelStyle(ConnectionStatus status)
{
    return ConnectionStatusLabelParams[(int)status].style;
}

ThorQ::AccountLoginWidget::AccountLoginWidget(ThorQ::AccountController* accountController, QWidget* parent)
	: QWidget(parent)
    , m_connectionStatus(ConnectionStatus::Error)
    , m_title(new QLabel(this))
    , m_onlineStatus(new QLabel(this))
    , m_usernameInput(new NamedLineEdit(this))
    , m_passwordInput(new NamedLineEdit(this))
    , m_loginButton(new QPushButton(this))
    , m_mainLayout(new QVBoxLayout(this))
    , m_headerLayout(new QHBoxLayout())
    , m_accountController(accountController)
{
    m_title->setText(THORQ_APPLICATION_NAME);
    m_title->setStyleSheet("font-size: 72px; color: #FFFFFF");

    m_onlineStatus->setText(ConnectionStatusLabelParams[(int)ConnectionStatus::Disconnected].text);
    m_onlineStatus->setStyleSheet(ConnectionStatusLabelParams[(int)ConnectionStatus::Disconnected].style);

    m_usernameInput->setName(tr("USERNAME"));
    m_usernameInput->setEchoMode(QLineEdit::EchoMode::Normal);

    m_passwordInput->setName(tr("PASSWORD"));
    m_passwordInput->setEchoMode(QLineEdit::EchoMode::Password);

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

    QObject::connect(m_loginButton, &QPushButton::clicked, [this]() {
        if (m_accountController != nullptr) {
            auto username = m_usernameInput->text();
            auto password = m_passwordInput->text();

            if (username.size() < THORQ_USERNAME_LEN_MIN || username.size() > THORQ_USERNAME_LEN_MAX ||
                password.size() < THORQ_PASSWORD_LEN_MIN || password.size() > THORQ_PASSWORD_LEN_MAX) {
                return;
            }

            m_passwordInput->setText("");
            m_accountController->login(username, password);
        }
    });

    updateUiState();
}

ThorQ::AccountLoginWidget::~AccountLoginWidget()
{
	delete m_headerLayout;
}

void ThorQ::AccountLoginWidget::setConnectionStatus(ConnectionStatus status)
{
    if (m_connectionStatus != status)
    {
        m_connectionStatus = status;
        m_onlineStatus->setText(getConnectionStatusLabelText(status));
        m_onlineStatus->setStyleSheet(getConnectionStatusLabelStyle(status));
        updateUiState();
    }
}

void ThorQ::AccountLoginWidget::updateUiState()
{
    if (m_connectionStatus == ConnectionStatus::Connected)
    {
        m_usernameInput->show();

        m_passwordInput->setText("");
        m_passwordInput->show();

        m_loginButton->show();
    }
    else
    {
        m_usernameInput->hide();
        m_passwordInput->hide();
        m_loginButton->hide();
    }
}
