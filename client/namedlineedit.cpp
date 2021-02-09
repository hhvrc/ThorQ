#include "namedlineedit.h"

#include "stylesheets.h"

NamedLineEdit::NamedLineEdit(QWidget* parent)
    : QWidget(parent)
    , m_label(new QLabel(this))
    , m_lineEdit(new QLineEdit(this))
    , m_layout(new QVBoxLayout(this))
{
    m_layout->addWidget(m_label);
    m_layout->addWidget(m_lineEdit);
    setLayout(m_layout);

    setStyleSheet(ThorQ::StyleSheets::tryGetStylesheet("namedlineedit"));

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
    m_label->setText(name);
}

void NamedLineEdit::setText(QString text)
{
    m_lineEdit->setText(text);
}

void NamedLineEdit::setEchoMode(QLineEdit::EchoMode echoMode)
{
    m_lineEdit->setEchoMode(echoMode);
}
