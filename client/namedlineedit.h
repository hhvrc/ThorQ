#ifndef NAMEDLINEEDIT_H
#define NAMEDLINEEDIT_H

#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QWidget>

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
    void editingFinished();
public slots:
    void setName(QString name);

    void setText(QString text);
    void clearText();

    void setError(QString error);
    void showError();
    void hideError();

    void setEchoMode(QLineEdit::EchoMode);
private:
    QLabel* m_labelTitle;
    QLineEdit* m_lineEdit;
    ErrorLabel* m_labelError;
    QVBoxLayout* m_layout;
};

#endif // NAMEDLINEEDIT_H
