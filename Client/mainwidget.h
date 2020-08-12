#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

#include <enums.h>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;

/**
 * @brief The MainWidget class
 */
class MainWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWidget)
public:
    MainWidget(QWidget* parent = nullptr);
    ~MainWidget() = default;
signals:
	void logoutButtonClicked();
	void usernameEntered(const QString& username);
public slots:
	void setLoginState(thorq_login_state_t state);
	void setSessionState(thorq_session_state_t state);
	void setConnectionPing(uint ping);
private slots:
	void updateUiState();
	void updateUiPing();
private:
	int m_state;
	uint m_ping;

	QPushButton* m_logoutButton;

	QVBoxLayout* m_vlayout;
	QHBoxLayout* m_hlayout;
};

#endif // MAINWIDGET_H
