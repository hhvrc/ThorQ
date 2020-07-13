#ifndef COMBINEWIDGET_H
#define COMBINEWIDGET_H

#include <QWidget>
#include <QDebug>

#include "enums.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;

class LoginWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(LoginWidget)
public:
    LoginWidget(QWidget* parent = nullptr);
    ~LoginWidget();
public slots:
    void SetState(qint16 state);
    void SetConnectionPing(qint64 ping);
signals:
    void LoginRequest(const QString& username);
private slots:
    void updateStatus();
private:
    qint16 m_state;
    qint64 m_ping;

    QLabel* m_title;
    QLabel* m_onlineStatus;
    QLineEdit* m_usernameInput;
    QPushButton* m_loginButton;

    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_headerLayout;
};

#endif // COMBINEWIDGET_H
