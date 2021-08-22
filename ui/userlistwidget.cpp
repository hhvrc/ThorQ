#include "userlistwidget.h"

#include <QScrollArea>

ThorQ::UI::UserListWidget::UserListWidget(QWidget* parent)
    : QWidget(parent)
	, m_scrollArea(new QScrollArea(this))
{
}
