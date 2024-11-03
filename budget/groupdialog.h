#pragma once

#ifndef GROUPDIALOG_H
#define GROUPDIALOG_H

#include "ui_groupdialog.h"

#include <QString>
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>

class GroupDialog : public QDialog
{
Q_OBJECT
public:
    GroupDialog(QWidget *parent=nullptr) : QDialog(parent)
    {
        ui.setupUi(this);
    }
    ~GroupDialog() 
    {}

    QString getNewGroupName() 
    {
        return ui.groupname_edit->text();
    }

    int getDirection()
    {
        return ui.income->isChecked() ? 1 : 0;
    }

private:
    Ui::GroupDialog ui;

};

#endif