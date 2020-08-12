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
	~LoginWidget() = default;
public slots:
    void setConnectionState(thorq_connection_state_t state);
    void setCryptoState(thorq_crypto_state_t state);
    void setAuthState(thorq_auth_state_t state);

    void setLoginState(thorq_login_state_t state);
    void setConnectionPing(uint ping);
signals:
    void regkeyEntered(const QString& username);
    void usernameEntered(const QString& username);
private slots:
    void updateUiState();
	void updateUiPing();
private:
    int m_state;
    uint m_ping;

	QLabel* m_title;
	QLabel* m_onlineStatus;
    QLineEdit* m_textInput;
    QPushButton* m_acceptButton;

	QVBoxLayout* m_mainLayout;
	QHBoxLayout* m_headerLayout;
};

#endif // LOGINWIDGET_H
