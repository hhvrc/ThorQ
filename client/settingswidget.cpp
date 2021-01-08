#include "settingswidget.h"

#include <constants.h>

ThorQ::SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent)
    , m_settings(new QSettings(this))
{
}

ThorQ::SettingsWidget::~SettingsWidget()
{
}

void ThorQ::SettingsWidget::cleanup()
{
    m_settings->clear();
}
