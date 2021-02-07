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
public slots:
    void setConnectionStatus(ConnectionStatus status);

	void setConnectionPing(uint ping);
private slots:
    void updateUiState();
	void updateUiPing();
private:
    ConnectionStatus m_connectionStatus;
    unsigned int m_ping;

	QLabel* m_title;
	QLabel* m_onlineStatus;
    QLineEdit* m_textInput;
    QPushButton* m_acceptButton;

	QVBoxLayout* m_mainLayout;
	QHBoxLayout* m_headerLayout;
};

#endif // LOGINWIDGET_H
