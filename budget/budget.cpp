#include "budget.h"

budget::budget(QWidget *parent)
    : QMainWindow(parent), _query(nullptr)
{
    QObject::connect(this, SIGNAL(user_exit()), this, SLOT(exitApplication()));
    ui.setupUi(this);

    ui.date_from->setDate(QDate(2000, 1, 1));
    ui.date_to->setDate(QDate::currentDate());

    db_conf.insert("db_type", "QPSQL");
    db_conf.insert("db_host", "192.168.37.207");
    db_conf.insert("db_port", "5433");
    db_conf.insert("db_name", "budget");
    db_conf.insert("db_user", "budget");
    db_conf.insert("db_pass", "budget");

    loadSettings();
    connectToDatabase();
    checkDatabase();

    groupmodel = new GroupModel(this);
    ui.group_list->setModel(groupmodel);

    setupGroupModel();
    updateModels();
    setupViewEdit();

    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(checkTimer()));
    timer.setSingleShot(false);
    timer.start(500);
}

budget::~budget()
{}

void budget::checkTimer()
{
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

void budget::loadSettings()
{
    QFile f("budget.ini");
    if (f.open(QIODevice::ReadOnly))
    {
        while (!f.atEnd())
        {
            QString str = f.readLine();
            QStringList lst = str.split("=");
            if (lst.count()==2)
            {
                QString key = lst.at(0);
                QString val = lst.at(1);
                db_conf.insert(key, val);
            }
        }
        f.close();
    }
}

void budget::connectToDatabase()
{
    QString db_type = db_conf.value("db_type", "QPSQL");
    QString db_host = db_conf.value("db_host");
    QString db_port = db_conf.value("db_port");
    QString db_name = db_conf.value("db_name");
    QString db_user = db_conf.value("db_user");
    QString db_pass = db_conf.value("db_pass");

    qDebug() << " ==================== DATABASE INITIALIZATION ==============";
    qDebug() << "\tTYPE: " << db_type;
    qDebug() << "\tHOST: " << db_host;
    qDebug() << "\tPORT: " << db_port;
    qDebug() << "\tNAME: " << db_name;
    qDebug() << "\tUSER: " << db_user;
    qDebug() << "\tPASS: " << db_pass;
    qDebug() << " ============================================================";

    db = QSqlDatabase::addDatabase(db_type, "budget");
    db.setHostName(db_host);
    db.setDatabaseName(db_name);
    db.setUserName(db_user);
    db.setPassword(db_pass);
    db.setPort(db_port.toInt());
    bool ok = db.open();

    if (ok)
    {
        qDebug() << "-- database is opened --";
        _query=new QSqlQuery(db);
    }
    else
    {
        qDebug() << "database NOT connected";
        qDebug() << db.lastError().text();
    }
}

bool budget::checkDatabase()
{
    bool retbool = false;
    if (!_query)
        return retbool;

#if 0
    _query->exec("DROP TABLE expenses");
    _query->exec("DROP TABLE expense_groups");
    _query->exec("DROP TABLE budget_config");
    _query->exec("DROP SEQUENCE uid_serial");
#endif

    // Checking database version
    QString str = "SELECT version()";
    if (_query->exec(str))
    {
        _query->next();
        qDebug() << "SQL server reports version: " << _query->value(0).toString();
    }
    else
    {
        qDebug() << "SQL server does not report version";
    }

    /* Checking if we can access our data inside
    *
    * We "recreate" the database in 2 ocassions: if we cannot get the version from the
    * budget table, meaning the database was never populated, or when the version is
    * smaller than the version burned into the application.
    */

    int recreate = 2;   // 0 - do not do anything, 1 - update structure, 2 - create everything
    int  app_db_ver  = 1;
    if (_query->exec("SELECT val FROM budget_config WHERE key='version'"))
    {
        int ver = _query->value(0).toInt();
        if (ver>=app_db_ver)
            recreate = 0;
        else
        {
            recreate = 1;
            qDebug() << "Recreation is necessary, since old version " << _query->value(0).toString() << " has been found!";
        }
    }
    else
        qDebug() << "Recreation is necessary, since no matching database structure was found";

    /* Here we do some "brute force" to push whatever possible into the DB, thus
     * we rely on the database itself to fail if an element is already there. It is
     * advised to fine-tune this approach to check for the existence of the structures or
     * elements.
     */

    if (recreate==2)
    {
        // create our uid sequence firts
        QString cmd = "CREATE SEQUENCE uid_serial INCREMENT 1 START 1";
        if (!_query->exec(cmd))
            qDebug() << "Sequence [uid_serial] cannot be created, since: " << _query->lastError().text();

        // let's create and populate the budget_config table
        cmd = "CREATE TABLE budget_config (uid INTEGER default nextval('uid_serial'), key VARCHAR(128), value VARCHAR(128))";
        if (_query->exec(cmd))
        {
            qDebug() << "budget_config table is created";
        }
        else
        {
            qDebug() << "Cannot create budget_config table since: " << _query->lastError().text();
        }

        _query->exec("DELETE FROM budget_config WHERE key='version'");

        if (_query->exec("INSERT INTO budget_config (key, value) VALUES('version','"+QString::number(app_db_ver)+"')"))
        {
            qDebug() << "registered new version " << app_db_ver;
        }

        // Create and populate the group that contains the income/expense groups
        QStringList ig_lst;
        ig_lst << "CREATE TABLE expense_groups (uid INTEGER default nextval('uid_serial'), name VARCHAR(128), direction INTEGER, PRIMARY KEY(uid))";
        ig_lst << "INSERT INTO expense_groups (name, direction) VALUES ('salary dad', 1)";
        ig_lst << "INSERT INTO expense_groups (name, direction) VALUES ('salary mom', 1)";
        ig_lst << "INSERT INTO expense_groups (name, direction) VALUES ('food', 0)";
        ig_lst << "INSERT INTO expense_groups (name, direction) VALUES ('car', 0)";
        ig_lst << "INSERT INTO expense_groups (name, direction) VALUES ('internet', 0)";
        ig_lst << "INSERT INTO expense_groupss (name, direction) VALUES ('electricity', 0)"; // Typo here, intentionally!

        // Create expenses table in which we store the incoming and outgoing money
        QString ec_str = "CREATE TABLE expenses (uid INTEGER default nextval('uid_serial'), date TIMESTAMP default now(), group_uid INTEGER, name VARCHAR(128), description VARCHAR(256), amount NUMERIC(8,2)";
        ec_str += ",pdf VARCHAR(256), pdf_hex VARCHAR(256)";
        ec_str += ",png VARCHAR(256), png_hex TEXT";
        ec_str += ",PRIMARY KEY(uid)";
        ec_str += ",CONSTRAINT fk_gid FOREIGN KEY(group_uid) REFERENCES expense_groups(uid))";
        qDebug() << "EC: " << ec_str;
        ig_lst << ec_str;

        // Create a some dummy entries for the expenses. We cheat a bit here, since we know that the sequence was
        // just created, thus

        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (2, '2024-01-01', 'Job Income', '1000')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (2, '2024-02-01', 'Job Income', '1000')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (2, '2024-03-01', 'Job Income', '1000')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (2, '2024-04-01', 'Job Income', '1000')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (2, '2024-05-01', 'Job Income', '1000')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (2, '2024-06-01', 'Job Income', '1000')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (2, '2024-07-01', 'Job Income', '1000')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (2, '2024-08-01', 'Job Income', '1000')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (2, '2024-09-01', 'Job Income', '1000')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (2, '2024-10-01', 'Job Income', '1000')";

        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (3, '2024-01-01', 'Job Income', '980')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (3, '2024-02-01', 'Job Income', '1002')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (3, '2024-03-01', 'Job Income', '1100')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (3, '2024-04-01', 'Job Income', '1200')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (3, '2024-05-01', 'Job Income', '1300')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (3, '2024-06-01', 'Job Income', '990')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (3, '2024-07-01', 'Job Income', '1111')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (3, '2024-08-01', 'Job Income', '1234.11')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (3, '2024-09-01', 'Job Income', '1000.00')";
        ig_lst << "INSERT INTO expenses (group_uid, date, name, amount) VALUES (3, '2024-10-01', 'Job Income', '1000.01')";


        // Execute the SQL commands

        for (int i=0;i<ig_lst.count();i++)
        {
            if (!_query->exec(ig_lst.at(i)))
            {
                qDebug() << "ERROR while executing: " << ig_lst.at(i) << " Message: " << _query->lastError().text();
            }
        }
    }
    else if (recreate==1)   // Refresh structure
    {
        // not implemented currently
    }
    return retbool;
}

void budget::closeDatabase()
{
    db.close();
}

void budget::on_add_group_clicked()
{
    GroupDialog* gd = new GroupDialog(this);
    if (gd->exec() == QDialog::Accepted)
    {
        QString newName = gd->getNewGroupName();
        int dir = gd->getDirection();
        if (!newName.isEmpty())
        {
            QString cmd = "INSERT INTO expense_groups (name, direction) VALUES (:name, :direction)";
            _query->prepare(cmd);
            _query->bindValue(":name", newName);
            _query->bindValue(":direction", dir);
            if (!_query->exec())
            {
                qDebug() << "Cannot add new group into DB, since: " << _query->lastError().text();
            }
            updateModels();
        }
    }
}

void budget::on_del_group_clicked()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Delete group?"));
    msgBox.setInformativeText(tr("Do you want to delete he selected group?"));
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setIcon(QMessageBox::Warning);
    int ret = msgBox.exec();

    if (ret != QMessageBox::Yes)
        return;

    QModelIndex idx = ui.group_list->selectionModel()->currentIndex();
    if (idx.isValid())
    {
        QString gid = groupmodel->getUid(idx).toString();
        QString cmd = "DELETE FROM expenses WHERE group_uid=:group_uid";
        _query->prepare(cmd);
        _query->bindValue(":group_uid", gid);
        if (!_query->exec())
        {
            qDebug() << "COULD NOT DELETE expense entries, since: " << _query->lastError().text();
        }

        cmd = "DELETE FROM expense_group WHERE uid=:uid";
        _query->prepare(cmd);
        _query->bindValue(":uid", gid);
        if (!_query->exec())
        {
            qDebug() << "COULD NOT DELETE group, since: " << _query->lastError().text();
        }
    }
    updateModels();
}

void budget::on_add_trans_clicked()
{
    QStringList gnames;
    if (_query->exec("SELECT name FROM expense_groups ORDER BY name ASC"))
    {
        while (_query->next())
            gnames << _query->value(0).toString();
    }

    ExpenseInfo* info = new ExpenseInfo();
    ExpenseDialog* ed = new ExpenseDialog(gnames, info, this);
    if (ed->exec() == QDialog::Accepted)
    {
        if (_query->exec("SELECT uid, direction from expense_groups WHERE name='" + info->group_name + "'"))
        {
            if (_query->next())
            {
                QString gid = _query->value(0).toString();
                int     dir = _query->value(1).toInt();
                if (!dir)
                    info->amount = -abs(info->amount);
                QString cmd = "INSERT INTO expenses (date, name, amount, group_uid) VALUES (:date, :name, :amount, :group_uid)";
                _query->prepare(cmd);
                _query->bindValue(":date", info->date);
                _query->bindValue(":name", info->name);
                _query->bindValue(":amount", info->amount);
                _query->bindValue(":group_uid", gid);
                if (!_query->exec())
                    qDebug() << "Could not insert new entry, since: " << _query->lastError().text();
            }
        }
        updateModels();
    }
}

void budget::setupGroupModel()
{
    budgetmodel = new BudgetModel(this);
    budgetmodel->setQuery(_query);
    budgetsortfiltermodel = new BudgetSortFilterModel(this);
    budgetsortfiltermodel->setSourceModel(budgetmodel);
    ui.table_trans->setModel(budgetsortfiltermodel);
    budgetmodel->updateFromDatabase();

    QObject::connect(ui.date_from, SIGNAL(dateChanged(QDate)), this, SLOT(dateChanged(QDate)));
    QObject::connect(ui.date_to, SIGNAL(dateChanged(QDate)), this, SLOT(dateChanged(QDate)));
    QObject::connect(ui.findSearch, SIGNAL(textActivated(const QString&)), this, SLOT(findTextChanged(const QString&)));

    updateModels();
}

void budget::setupViewEdit()
{
    ui.table_trans->setItemDelegate(new BudgetDelegate);
    ui.table_trans->setEditTriggers(QAbstractItemView::DoubleClicked
        | QAbstractItemView::SelectedClicked);
}

void budget::dateChanged(QDate date)
{
    updateModels();
}

void budget::findTextChanged(const QString& text)
{
    budgetsortfiltermodel->setFindText(text);
    updateModels();
}

void budget::updateModels()
{
    budgetsortfiltermodel->setDateFromTo(ui.date_from->date(), ui.date_to->date());
    budgetsortfiltermodel->invalidate();
    groupmodel->updateFromDatabase(_query);
    budgetmodel->updateFromDatabase();

    qreal income  = 0;
    qreal expense = 0;
    
    int exp_rc = budgetsortfiltermodel->rowCount();
    for (int i = 0; i < exp_rc; i++)
    {
        QModelIndex index = budgetsortfiltermodel->index(i, 3);
        qreal amount = budgetsortfiltermodel->data(index).toReal();
        if (amount < 0)
            expense += amount;
        else
            income += amount;
    }
    ui.balance->setText(QString::number(income+expense));
    ui.total_income->setText(QString::number(income));
    ui.total_expenses->setText(QString::number(expense));
}
