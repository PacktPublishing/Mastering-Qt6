#pragma once

#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include <QString>
#include <QStringList>
#include <QAbstractListModel>
#include <QSqlQuery>
#include <QSqlError>

class GroupInfo
{
public:
	int     uid;
	QString name;
	int		direction;
};

class GroupModel : public QAbstractListModel
{
	Q_OBJECT
public:
	GroupModel(QObject* parent = nullptr) : QAbstractListModel(parent)
	{}
	~GroupModel() {}

	int rowCount(const QModelIndex& parent = QModelIndex()) const
	{
		return entries.count();
	}

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
	{
		if (role == Qt::DisplayRole && orientation==Qt::Horizontal)
		{
			return QVariant("Groupname");
		}
		return QVariant();
	}

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
	{
		if (!index.isValid())
			return QVariant();
		if (role == Qt::DisplayRole)
		{
			int r = index.row();
			GroupInfo *e = entries.at(r);
				return e->name;
		}
		return QVariant();
	}

	QVariant getUid(const QModelIndex& index)
	{
		if (!index.isValid())
			return QVariant();
		int r = index.row();
		GroupInfo* e = entries.at(r);
		return e->uid;
	}

	void updateFromDatabase(QSqlQuery *query)
	{
		if (!query) return;
		beginResetModel();
		clear();
		QString cmd = "SELECT uid, name, direction FROM expense_groups";
		if (query->exec(cmd))
		{
			while (query->next())
			{
				GroupInfo* entry = new GroupInfo();
				entries.append(entry);
				entry->uid = query->value(0).toInt();
				entry->name = query->value(1).toString();
				entry->direction = query->value(2).toInt();
			}
		}
		else
		{
			qDebug() << "Cannot fetct entries from DB, since: " << query->lastError().text();
		}
		endResetModel();
	}

private:
	void clear()
	{
		for (int i = 0; i < entries.count(); i++)
			delete(entries.at(i));
		entries.clear();
	}


private:
	QList<GroupInfo*> entries;


};

#endif