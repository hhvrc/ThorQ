#ifndef USERLISTMODEL_H
#define USERLISTMODEL_H

#include <QAbstractListModel>

class Account;

class UserListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(UserListModel)
public:
    UserListModel(QObject* parent = nullptr);
protected:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
private:
    QList<Account*> m_users;
};

#endif // USERLISTMODEL_H
