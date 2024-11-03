#pragma once

#ifndef EXPENSEINFO_H
#define EXPENSEINFO_H

#include <QString>

class ExpenseInfo
{
public:
	int     uid;
	int		group_uid;
	QString date;
	QString name;
	QString group_name;
	QString description;
	qreal   amount;
};

#endif