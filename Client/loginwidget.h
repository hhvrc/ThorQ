#ifndef COMBINEWIDGET_H
#define COMBINEWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;

class LoginWidget : public QWidget
{
public:
    LoginWidget();
private:
    QLabel* m_title;
    QLabel* m_onlineStatus;
    QPushButton* m_loginButton;
    QVBoxLayout* m_vlayout;
    QHBoxLayout* m_hlayout;
};

#endif // COMBINEWIDGET_H
