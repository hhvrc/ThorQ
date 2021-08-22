#include "settingswidget.h"

#include <QSettings>

ThorQ::UI::SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent)
    , m_settings(new QSettings(this))
{
}

ThorQ::UI::SettingsWidget::~SettingsWidget()
{
}

void ThorQ::UI::SettingsWidget::cleanup()
{
    m_settings->clear();
}
