#ifndef USERDELEGATE_H
#define USERDELEGATE_H

#include <QStyledItemDelegate>

class UserDelegate : public QStyledItemDelegate
{
	Q_OBJECT
	Q_DISABLE_COPY(UserDelegate)
public:
	UserDelegate(QWidget* parent = nullptr);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
signals:
public slots:
private slots:
private:
};

#endif // USERDELEGATE_H
