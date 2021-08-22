#ifndef USERLISTITEM_H
#define USERLISTITEM_H

#include <QGraphicsItem>

class QString;
class QGraphicsTextItem;
class QGraphicsPixmapItem;
class QGraphicsPixmapItem;
class QGraphicsRectItem;

namespace ThorQ::UI {

class UserListItem : public QGraphicsItem
{
public:
	UserListItem(const QString& username, QGraphicsItem* parent = nullptr);
private:
	QGraphicsTextItem* m_usernameText;
	QGraphicsPixmapItem* m_collarIcon;
	QGraphicsPixmapItem* m_vrIcon;
	QGraphicsRectItem* m_requestButton;
	QGraphicsRectItem* m_acceptButton;
	QGraphicsRectItem* m_denyButton;
};

}

#endif // USERLISTITEM_H
