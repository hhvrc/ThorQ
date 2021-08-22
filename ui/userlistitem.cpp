#include "userlistitem.h"

#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>

ThorQ::UI::UserListItem::UserListItem(const QString& username, QGraphicsItem* parent)
	: QGraphicsItem(parent)
	, m_usernameText(new QGraphicsTextItem(username, this))
	, m_collarIcon(new QGraphicsPixmapItem(this))
	, m_vrIcon(new QGraphicsPixmapItem(this))
	, m_requestButton(new QGraphicsRectItem(this))
	, m_acceptButton(new QGraphicsRectItem(this))
	, m_denyButton(new QGraphicsRectItem(this))
{
}
