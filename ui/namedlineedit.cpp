#include "namedlineedit.h"

#include "errorlabel.h"
#include "stylesheets.h"

#include <QVBoxLayout>
#include <QCryptographicHash>

ThorQ::UI::NamedLineEdit::NamedLineEdit(QWidget* parent)
    : QWidget(parent)
    , m_labelTitle(new QLabel(this))
    , m_lineEdit(new QLineEdit(this))
    , m_labelError(new ErrorLabel(this))
    , m_layout(new QVBoxLayout(this))
    , m_simpleText(false)
    , m_inputValidator(ThorQ::Validators::FunctionPlaceholder)
    , m_lastValueLength(0)
    , m_lastValueHash()
{
    setStyleSheet(ThorQ::StyleSheets::tryGetStylesheet("namedlineedit"));

    m_labelError->hide();

    m_layout->setAlignment(Qt::AlignVCenter);
    m_layout->addWidget(m_labelTitle);
    m_layout->addWidget(m_lineEdit);
    m_layout->addWidget(m_labelError);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    QObject::connect(m_lineEdit, &QLineEdit::editingFinished, this, &ThorQ::UI::NamedLineEdit::handleEditingFinished);
}

ThorQ::UI::NamedLineEdit::~NamedLineEdit()
{
}

QString ThorQ::UI::NamedLineEdit::text() const
{
    return m_lineEdit->text();
}

void ThorQ::UI::NamedLineEdit::setName(QString name)
{
    m_labelTitle->setText(name);
}

void ThorQ::UI::NamedLineEdit::setText(QString text)
{
    m_lineEdit->setText(text);
}

void ThorQ::UI::NamedLineEdit::clearText()
{
    m_lineEdit->setText("");
}

void ThorQ::UI::NamedLineEdit::showError(QString error)
{
    m_labelError->setText(error);
    m_labelError->show();
    adjustSize();
}

void ThorQ::UI::NamedLineEdit::showError()
{
    m_labelError->show();
    adjustSize();
}

void ThorQ::UI::NamedLineEdit::hideError()
{
    m_labelError->hide();
    adjustSize();
}

void ThorQ::UI::NamedLineEdit::setEchoMode(QLineEdit::EchoMode echoMode)
{
    m_lineEdit->setEchoMode(echoMode);
}

void ThorQ::UI::NamedLineEdit::setSimpleText(bool isSimple)
{
    m_simpleText = isSimple;
}

void ThorQ::UI::NamedLineEdit::setInputValidator(ThorQ::Validators::FuncType validator)
{
    m_inputValidator = validator;
}

void ThorQ::UI::NamedLineEdit::handleEditingFinished()
{
    emit editingFinished();

    QString text = m_lineEdit->text();
    if (m_simpleText) {
        text = text.simplified();
        m_lineEdit->setText(text);
    }

    auto validation = m_inputValidator(text);

    if (!validation.first) {
        showError(validation.second);
        return;
    }

    if (m_lastValueLength == text.length()) {

        QByteArray textUtf8 = text.toUtf8();
        QCryptographicHash hasher(QCryptographicHash::Sha256);
        hasher.addData(textUtf8);
        QByteArray hash = hasher.result();

        if (m_lastValueHash == hash) {
            return;
        }
        m_lastValueHash = hash;
    }
    m_lastValueLength = text.length();

    emit textChanged();
}
