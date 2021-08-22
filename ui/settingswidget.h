#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>

class QSettings;

namespace ThorQ::UI {

class SettingsWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SettingsWidget)
public:
    SettingsWidget(QWidget* parent = nullptr);
    ~SettingsWidget();

public slots:
    void cleanup();
private:
    QSettings* m_settings;
};

}

#endif // SETTINGSWIDGET_H
