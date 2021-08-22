#ifndef NAMEDLINEEDIT_H
#define NAMEDLINEEDIT_H

#include <QWidget>
#include <QString>
#include <QLineEdit>
#include <QByteArray>

#include "validators.h"

class QLabel;
class QVBoxLayout;

namespace ThorQ::UI {

class ErrorLabel;

class NamedLineEdit : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(NamedLineEdit)
public:
    NamedLineEdit(QWidget* parent = nullptr);
    ~NamedLineEdit();

    QString text() const;
signals:
    void textChanged();
    void editingFinished();
public slots:
    void setName(QString name);

    void setText(QString text);
    void clearText();

    void showError(QString error);
    void showError();
    void hideError();

    void setEchoMode(QLineEdit::EchoMode);

    void setSimpleText(bool isSimple);
    void setInputValidator(ThorQ::Validators::FuncType inputValidator);
private slots:
    void handleEditingFinished();
private:
    QLabel* m_labelTitle;
    QLineEdit* m_lineEdit;
    ErrorLabel* m_labelError;
    QVBoxLayout* m_layout;

    bool m_simpleText;
    ThorQ::Validators::FuncType m_inputValidator;

    int m_lastValueLength;
    QByteArray m_lastValueHash;
};

}

#endif // NAMEDLINEEDIT_H
