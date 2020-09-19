#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

#include <enums.h>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QGraphicsScene;

class QListView;
class UserListModel;

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

    void updateUser(const QString& username, std::uint8_t state);
    void removeUser(const QString& username);
    void clearUsers();
private slots:
	void updateUiState();
	void updateUiPing();
private:
	int m_state;
    quint16 m_ping;

    QPushButton* m_logoutButton;

    QGraphicsScene* m_userScene;

	QVBoxLayout* m_vlayout;
	QHBoxLayout* m_hlayout;
};

#endif // MAINWIDGET_H
