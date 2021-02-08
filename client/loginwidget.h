#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

#include "enums.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;

/**
 * @brief The LoginWidget class
 */
class LoginWidget : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(LoginWidget)
public:
	LoginWidget(QWidget* parent = nullptr);
	~LoginWidget();
signals:
	void usernameEntered(const QString& username);
    void passwordEntered(const QString& username);
public slots:
    void setConnectionStatus(ConnectionStatus status);
private slots:
    void updateUiState();
private:
    ConnectionStatus m_connectionStatus;

	QLabel* m_title;
	QLabel* m_onlineStatus;
    QLineEdit* m_textInput;
    QPushButton* m_loginButton;
    QPushButton* m_forgotButton;
    QPushButton* m_registerButton;

	QVBoxLayout* m_mainLayout;
	QHBoxLayout* m_headerLayout;
    QHBoxLayout* m_belowLoginLayout;
};

#endif // LOGINWIDGET_H
