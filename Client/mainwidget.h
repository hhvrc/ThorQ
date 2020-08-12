#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

#include <enums.h>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QListView;

class UserModel;
class UserDelegate;

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
	void setLoginState(THORQ_STATE_LOGIN state);
	void setSessionState(THORQ_STATE_SESSION state);
	void setConnectionPing(uint ping);
private slots:
	void updateUiState();
	void updateUiPing();
private:
	int m_state;
	uint m_ping;

	QPushButton* m_logoutButton;
	QListView*   m_listView;

	UserModel* m_userModel;
	UserDelegate* m_userDelegate;

	QVBoxLayout* m_vlayout;
	QHBoxLayout* m_hlayout;
};

#endif // MAINWIDGET_H
