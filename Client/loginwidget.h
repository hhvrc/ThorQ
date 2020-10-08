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
    void setConnectionState(THORQ_STATE_CONNECTION state);
    void setCryptoState(THORQ_STATE_CRYPTO state);
    void setAuthState(THORQ_STATE_AUTH state);

    void setLoginState(THORQ_STATE_LOGIN state);
	void setConnectionPing(uint ping);
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
