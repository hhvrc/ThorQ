#ifndef COMBINEWIDGET_H
#define COMBINEWIDGET_H

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
public slots:
	void SetConnectionState(thorq_connection_state_t state);
    void SetCryptoState(thorq_crypto_state_t state);
    void SetAuthState(thorq_auth_state_t state);

	void SetLoginState(thorq_login_state_t state);
	void SetConnectionPing(uint ping);
signals:
	void LoginRequest(const QString& username);
private slots:
    void updateUiState();
	void updateUiPing();
private:
    int m_state;
	uint m_ping;

	QLabel* m_title;
	QLabel* m_onlineStatus;
	QLineEdit* m_usernameInput;
	QPushButton* m_loginButton;

	QVBoxLayout* m_mainLayout;
	QHBoxLayout* m_headerLayout;
};

#endif // COMBINEWIDGET_H
