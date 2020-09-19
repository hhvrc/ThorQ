#include "userlistwidget.h"

#include <QScrollArea>

UserListWidget::UserListWidget(QWidget* parent)
    : QWidget(parent)
	, m_scrollArea(new QScrollArea(this))
{
}
