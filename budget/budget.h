#pragma once

#ifndef BUDGET_H
#define BUDGET_H

#include "ui_budget.h"
#include "budgetmodel.h"
#include "groupmodel.h"
#include "expensedialog.h"
#include "groupdialog.h"

#include <QtWidgets/QMainWindow>

#include <QHash>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSortFilterProxyModel>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QTimer>

class budget : public QMainWindow
{
    Q_OBJECT

public:
    budget(QWidget *parent = nullptr);
    ~budget();

signals:
    void user_exit();

public slots:
    void exitApplication()
    {
        qApp->exit(0);
    }

    void loadSettings();
    void connectToDatabase();
    bool checkDatabase();
    void closeDatabase();

    void on_add_group_clicked();
    void on_del_group_clicked();
    void on_add_trans_clicked();

    void setupGroupModel();
    void findTextChanged(const QString& text);
    void dateChanged(QDate date);
    void updateModels();

    void checkTimer();
    void setupViewEdit();

private:
    Ui::Budget ui;

    QHash<QString, QString> db_conf;
    QSqlDatabase db;
    QSqlQuery *_query;
    
    GroupModel* groupmodel;
    BudgetModel* budgetmodel;
    BudgetSortFilterModel* budgetsortfiltermodel;

    QTimer timer;


};

#endif
