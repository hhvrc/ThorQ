#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;

namespace Ui {
class ControlWidget;
}

/**
 * @brief The OverlayWidget class
 */
class OverlayWidget : public QWidget
{
	Q_OBJECT

public:
    /**
     * @brief OverlayWidget
     * @param parent
     */
	OverlayWidget(QWidget *parent = nullptr);
	~OverlayWidget();
private:
};

#endif // OVERLAYWIDGET_H
