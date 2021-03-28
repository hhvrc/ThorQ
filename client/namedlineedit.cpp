#include "namedlineedit.h"

#include "errorlabel.h"
#include "stylesheets.h"

NamedLineEdit::NamedLineEdit(QWidget* parent)
    : QWidget(parent)
    , m_labelTitle(new QLabel(this))
    , m_lineEdit(new QLineEdit(this))
    , m_labelError(new ErrorLabel(this))
    , m_layout(new QVBoxLayout(this))
{
    setStyleSheet(ThorQ::StyleSheets::tryGetStylesheet("namedlineedit"));

    m_labelError->hide();

    m_layout->setAlignment(Qt::AlignVCenter);
    m_layout->addWidget(m_labelTitle);
    m_layout->addWidget(m_lineEdit);
    m_layout->addWidget(m_labelError);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    QObject::connect(m_lineEdit, &QLineEdit::editingFinished, [this]()
    {
        emit editingFinished();
    });
}

NamedLineEdit::~NamedLineEdit()
{
}

QString NamedLineEdit::text() const
{
    return m_lineEdit->text();
}

void NamedLineEdit::setName(QString name)
{
    m_labelTitle->setText(name);
}

void NamedLineEdit::setText(QString text)
{
    m_lineEdit->setText(text);
}

void NamedLineEdit::clearText()
{
    m_lineEdit->setText("");
}

void NamedLineEdit::setError(QString error)
{
    m_labelError->setText(error);
    m_labelError->show();
    adjustSize();
}

void NamedLineEdit::clearError()
{
    m_labelError->setText("");
    m_labelError->hide();
    adjustSize();
}

void NamedLineEdit::setEchoMode(QLineEdit::EchoMode echoMode)
{
    m_lineEdit->setEchoMode(echoMode);
}
