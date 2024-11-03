#pragma once

#ifndef BUDGETMODEL_H
#define BUDGETMODEL_H

#include "expenseinfo.h"

#include <QString>
#include <QStringList>
#include <QAbstractTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSortFilterProxyModel>
#include <QDate>

class BudgetModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	BudgetModel(QObject* parent = nullptr) : QAbstractTableModel(parent)
	{}
	~BudgetModel() 
	{
		clear();
	}

	int rowCount(const QModelIndex& parent = QModelIndex()) const
	{
		return entries.count();
	}

	int columnCount(const QModelIndex& parent = QModelIndex()) const
	{
		return 4;
	}

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
	{
		if (role == Qt::DisplayRole && orientation==Qt::Horizontal)
		{
			switch (section)
			{
			case 0:
				return QVariant("Group");
				break;
			case 1:
				return QVariant("Date");
				break;
			case 2:
				return QVariant("Name");
				break;
			case 3:
				return QVariant("Amount");
				break;
			}
		}
		return QVariant();
	}

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
	{
		if (!index.isValid())
			return QVariant();
		if (role == Qt::DisplayRole)
		{
			int c = index.column();
			int r = index.row();
			ExpenseInfo* e = entries.at(r);
			switch (c)
			{
				case 0: return e->group_name; break;
				case 1: return e->date; break;
				case 2: return e->name; break;
				case 3: return e->amount; break;
				default: break;
			}
		}
		return QVariant();
	}

	void updateFromDatabase(QSqlQuery *query)
	{
		if (!query) return;
		beginResetModel();
		clear();
		QString cmd = "SELECT e.uid, g.name, g.uid, e.date, e.name, e.amount, e.description FROM expenses AS e, expense_groups g WHERE g.uid = e.group_uid ORDER BY e.date";
		if (query->exec(cmd))
		{
			while (query->next())
			{
				ExpenseInfo* entry = new ExpenseInfo();
				entries.append(entry);
				entry->uid = query->value(0).toInt();
				entry->group_name = query->value(1).toString();
				entry->group_uid= query->value(2).toInt();
				entry->date = query->value(3).toString();
				entry->name = query->value(4).toString();
				entry->amount = query->value(5).toReal();
				entry->description = query->value(6).toString();
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
	QList<ExpenseInfo*> entries;
};

class BudgetSortFilterModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
	BudgetSortFilterModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent)
	{}
	~BudgetSortFilterModel()
	{}

	void setDateFromTo(QDate from, QDate to)
	{
		date_from = from;
		date_to = to;
	}

	void setFindText(const QString& text)
	{
		beginResetModel();
		findtext = text;
		endResetModel();
	}

	bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
	{
		// date is in yyyy-MM-hh format 
		QModelIndex index2 = sourceModel()->index(sourceRow, 2, sourceParent);
		QString namedata = sourceModel()->data(index2).toString();
		if (findtext.isEmpty() || namedata.contains(findtext))
		{
			QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);
			QString datedata = sourceModel()->data(index1).toString().mid(0, 10);
			QDate rdate = QDate::fromString(datedata, "yyyy-MM-dd");
			if (date_from <= rdate && rdate <= date_to)
			{
				return true;
			}
		}
		return false;
	}

private:
	QDate date_from;
	QDate date_to;
	QString findtext;
};


#endif