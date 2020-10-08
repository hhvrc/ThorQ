#include "userlistmodel.h"

#include "user.h"

UserListModel::UserListModel(QObject *parent)
	: QAbstractListModel(parent)
{

}

int UserListModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return m_users.count();
}

QVariant UserListModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

//	if (index.row() < m_users.size())
//		return m_users.at(index.row());

	return QVariant();
}
