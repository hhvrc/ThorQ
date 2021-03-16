#include "accountrecoverwidget.h"

#include "accountcontroller.h"
#include "namedlineedit.h"

#include <constants.h>

#include <QPushButton>

ThorQ::AccountRecoverWidget::AccountRecoverWidget(ThorQ::AccountController* accountController, QWidget* parent)
    : QWidget(parent)
    , m_goBackButton(new QPushButton(this))
    , m_emailInput(new NamedLineEdit(this))
    , m_recoverButton(new QPushButton(this))
    , m_layout(new QVBoxLayout(this))
    , m_accountController(accountController)
{
    m_goBackButton->setText(tr("Go back"));
    m_goBackButton->setCursor(Qt::PointingHandCursor);

    m_emailInput->setName(tr("EMAIL"));
    m_emailInput->setEchoMode(QLineEdit::EchoMode::Normal);

    m_recoverButton->setText(tr("Recover"));
    m_recoverButton->setCursor(Qt::PointingHandCursor);

    m_layout->setContentsMargins(12, 12, 12, 12);
    m_layout->addWidget(m_goBackButton);
    m_layout->addWidget(m_emailInput);
    m_layout->addWidget(m_recoverButton);

    setLayout(m_layout);

    QObject::connect(m_goBackButton, &QPushButton::clicked, this, &QWidget::hide);
    QObject::connect(m_goBackButton, &QPushButton::clicked, [this](){ emit goBackButtonPressed(); });
    QObject::connect(m_recoverButton, &QPushButton::clicked, [this]() {
        if (m_accountController != nullptr) {
            auto email = m_emailInput->text();

            if (email.size() < THORQ_EMAIL_LEN_MIN || email.size() > THORQ_EMAIL_LEN_MAX) {
                return;
            }

            m_accountController->recoverAccount(email);
        }
    });
}

ThorQ::AccountRecoverWidget::~AccountRecoverWidget()
{

}
