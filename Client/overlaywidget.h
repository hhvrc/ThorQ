#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QWidget>
#include <QImage>
#include <QLabel>

namespace Ui {
class OverlayWidget;
}

class OverlayWidget : public QWidget
{
	Q_OBJECT

public:
	OverlayWidget(QWidget *parent = nullptr);
	~OverlayWidget();
private:
};

#endif // OVERLAYWIDGET_H
