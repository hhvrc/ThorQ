#ifndef RECOVERWIDGET_H
#define RECOVERWIDGET_H

#include <QWidget>

class QPushButton;
class QVBoxLayout;

namespace ThorQ::UI {

class NamedLineEdit;

class RecoverWidget : public QWidget
{
     Q_OBJECT
public:
    RecoverWidget(QWidget* parent = nullptr);
    ~RecoverWidget();
signals:
    void emailChanged(const QString& email);
    void goBackClicked();
    void recoverClicked();
public slots:
    void invalidateEmail(const QString& reason);
    void clearForms();
private slots:
    void handleGoBackClicked();
    void handleRecoverClicked();
    void handleEmailInputChanged();
private:
    QPushButton* m_goBackButton;

    NamedLineEdit* m_emailInput;
    QPushButton* m_recoverButton;

    QVBoxLayout* m_layout;
};

}

#endif // RECOVERWIDGET_H
