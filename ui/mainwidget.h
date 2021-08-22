#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

class QGraphicsScene;
class QVBoxLayout;
class QHBoxLayout;

namespace ThorQ::UI {

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
