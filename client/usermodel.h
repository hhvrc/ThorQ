#ifndef USERMODEL_H
#define USERMODEL_H

#include <QAbstractListModel>

#include "userdata.h"

class UserModel : public QAbstractListModel
{
	Q_OBJECT
	Q_DISABLE_COPY(UserModel)
public:
	UserModel(QObject* parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
signals:
public slots:
private slots:
private:
	QList<UserData> m_users;
};

#endif // USERMODEL_H
