#include "recoverwidget.h"

#include "namedlineedit.h"
#include "validators.h"

#include <QPushButton>
#include <QVBoxLayout>

ThorQ::UI::RecoverWidget::RecoverWidget(QWidget* parent)
    : QWidget(parent)
    , m_goBackButton(new QPushButton(this))
    , m_emailInput(new NamedLineEdit(this))
    , m_recoverButton(new QPushButton(this))
    , m_layout(new QVBoxLayout(this))
{
    // Go back button
    m_goBackButton->setText(tr("Go back"));
    m_goBackButton->setCursor(Qt::PointingHandCursor);
    QObject::connect(m_goBackButton, &QPushButton::clicked, this, &ThorQ::UI::RecoverWidget::handleGoBackClicked);

    // Recover button
    m_recoverButton->setText(tr("Recover"));
    m_recoverButton->setCursor(Qt::PointingHandCursor);
    QObject::connect(m_recoverButton, &QPushButton::clicked, this, &ThorQ::UI::RecoverWidget::handleRecoverClicked);

    // Email input
    m_emailInput->setName(tr("EMAIL"));
    m_emailInput->setEchoMode(QLineEdit::EchoMode::Normal);
    m_emailInput->setSimpleText(true);
    m_emailInput->setInputValidator(ThorQ::Validators::EmailValidator);
    QObject::connect(m_emailInput, &ThorQ::UI::NamedLineEdit::textChanged, m_emailInput, &ThorQ::UI::NamedLineEdit::hideError);
    QObject::connect(m_emailInput, &ThorQ::UI::NamedLineEdit::textChanged, this, &ThorQ::UI::RecoverWidget::handleEmailInputChanged);

    // Layout
    m_layout->setContentsMargins(12, 12, 12, 12);
    m_layout->addWidget(m_goBackButton);
    m_layout->addWidget(m_emailInput);
    m_layout->addWidget(m_recoverButton);
    setLayout(m_layout);

}

ThorQ::UI::RecoverWidget::~RecoverWidget()
{
}

void ThorQ::UI::RecoverWidget::invalidateEmail(const QString& reason)
{
    m_emailInput->showError(reason);
}

void ThorQ::UI::RecoverWidget::clearForms()
{
    m_emailInput->clearText();
}

void ThorQ::UI::RecoverWidget::handleGoBackClicked()
{
    emit goBackClicked();
}

void ThorQ::UI::RecoverWidget::handleRecoverClicked()
{
    emit recoverClicked();
}

void ThorQ::UI::RecoverWidget::handleEmailInputChanged()
{
    emit emailChanged(m_emailInput->text());
}
