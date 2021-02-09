#ifndef NAMEDLINEEDIT_H
#define NAMEDLINEEDIT_H

#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QWidget>

class NamedLineEdit : public QWidget
{
    Q_OBJECT
public:
    NamedLineEdit(QWidget* parent = nullptr);
    ~NamedLineEdit();

    QString text() const;
signals:
    void editingFinished();
public slots:
    void setName(QString name);
    void setText(QString text);
    void setEchoMode(QLineEdit::EchoMode);
private:
    QLabel* m_label;
    QLineEdit* m_lineEdit;
    QVBoxLayout* m_layout;
};

#endif // NAMEDLINEEDIT_H
