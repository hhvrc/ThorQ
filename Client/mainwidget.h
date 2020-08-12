#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

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
private:
    QVBoxLayout m_vlayout;
	QHBoxLayout m_hlayout;
};

#endif // MAINWIDGET_H
