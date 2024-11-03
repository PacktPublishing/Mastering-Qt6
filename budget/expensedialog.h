#pragma once

#ifndef EXPENSEDIALOG_H
#define EXPENSEDIALOG_H

#include "ui_expensedialog.h"
#include "expenseinfo.h"

#include <QString>
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QDateTimeEdit>

class ExpenseDialog : public QDialog
{
Q_OBJECT
public:
    ExpenseDialog(QStringList _group_names, ExpenseInfo *_info, QWidget *parent=nullptr) : QDialog(parent)
    {
        info = _info;
        group_names = _group_names;
        ui.setupUi(this);

        ui.group_box->insertItems(0, group_names);
    }
    ~ExpenseDialog()
    {}

public slots:
    void accept()
    {
        if (info)
        {
            info->name = ui.name_edit->text();
            info->amount = ui.amount_edit->text().toDouble();
            info->date = ui.date_edit->date().toString("yyyy-MM-dd");
            info->group_name = ui.group_box->currentText();
        }
        QDialog::accept();
    }
    
private:
    ExpenseInfo* info;
    QStringList group_names;
    Ui::ExpenseDialog ui;
};

#endif