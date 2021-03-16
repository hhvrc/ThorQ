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

namespace ThorQ {
class MainWidget : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(MainWidget)
public:
	MainWidget(QWidget* parent = nullptr);
    ~MainWidget() = default;
private:
    QGraphicsScene* m_userScene;

	QVBoxLayout* m_vlayout;
	QHBoxLayout* m_hlayout;
};
}

#endif // MAINWIDGET_H
