#include "server.h"

void Server::setupTCPServer()
{
    tcpserver = new QTcpServer(this);
    // Listening on the 12345 port for incoming client connections
    tcpserver->listen(QHostAddress::Any, 12345);
    QObject::connect(tcpserver, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
}

void Server::handleNewConnection()
{
    // Progress all not-yet handed connections
    while (tcpserver->hasPendingConnections())
    {
        // get the next pending connection's socket
        QTcpSocket* client = tcpserver->nextPendingConnection();

        // Assign an unique index for easier identification later on
        // we set the socket property to idx and also put into the hash container 
        // with the idx's value key for fast lookup
        int idx = client_index++;
        client->setProperty("id", idx);
        clients.insert(idx, client);

        // Put some message that we accepted a new connection
        qDebug() << "Socket is connected with id #" << idx << " " << client->peerAddress().toString();

        // Also connect signal of readyRead which tells us when there is a new data from the client
        QObject::connect(client, SIGNAL(readyRead()), this, SLOT(handleNewDataFromSocket()));

        // Connect this socket to the common handling of lost connections
        QObject::connect(client, SIGNAL(connectionClosed()), this, SLOT(handleSocketClosed()));
    }
}

void Server::handleNewDataFromSocket()
{
    if (QTcpSocket* client = qobject_cast<QTcpSocket*>(sender()))
    {
        // get Id for the socket over property
        int idx = client->property("id").toInt();

        // Read all data from the socket (what the client has sent)
        QByteArray ba = client->readAll();

        // We iterate over all elements (all clients) in the clients list
        QHashIterator<int, QTcpSocket*> it(clients);
        while (it.hasNext())
        {
            it.next();
            QTcpSocket* client_from_list = it.value();
            // We send the data to all clients expect who sent it
            if (client_from_list->property("id").toInt() != idx)
            {
                client_from_list->write(ba);
            }
        }
    }
}

void Server::handleSocketClosed()
{
    if (QTcpSocket* client = qobject_cast<QTcpSocket*>(sender()))
    {
        // Retrive the idx ftom the socket
        int idx = client->property("id").toInt();

        // Print out that we lost this connection
        qDebug() << "Socket #" << idx << " is closed";

        // Remove the socket from the list - this does not delete the instance
        clients.remove(idx);

        // Delete the instance with deleteLater. free() could make troubles in the event loop where
        // the eventloop would process invalid pointer of the socket. 
        client->deleteLater();
    }
}

void Server::releaseAllConnections()
{
    QHashIterator<int, QTcpSocket*> it(clients);
    while (it.hasNext())
    {
        it.next();
        QTcpSocket* client_from_list = it.value();
        client_from_list->close();
        client_from_list->deleteLater();
    }
    clients.clear();
}

void Server::setupDatabase()
{
    QHash<QString, QString> db_conf;
    QFile f("server.ini");
    if (f.open(QIODevice::ReadOnly))
    {
        while (!f.atEnd())
        {
            QString str = f.readLine();
            QStringList lst = str.split("=");
            if (lst.count() == 2)
            {
                QString key = lst.at(0);
                QString val = lst.at(1);
                db_conf.insert(key, val);
            }
        }
        f.close();
    }

    QString db_type = db_conf.value("db_type", "QPSQL");
    QString db_host = db_conf.value("db_host", "192.168.37.207");
    QString db_port = db_conf.value("db_port", "5433");
    QString db_name = db_conf.value("db_name", "budget");
    QString db_user = db_conf.value("db_user", "budget");
    QString db_pass = db_conf.value("db_pass", "budget");

    qDebug() << " ==================== DATABASE INITIALIZATION ==============";
    qDebug() << "\tTYPE: " << db_type;
    qDebug() << "\tHOST: " << db_host;
    qDebug() << "\tPORT: " << db_port;
    qDebug() << "\tNAME: " << db_name;
    qDebug() << "\tUSER: " << db_user;
    qDebug() << "\tPASS: " << db_pass;
    qDebug() << " ============================================================";

    db = QSqlDatabase::addDatabase(db_type, "server");
    db.setHostName(db_host);
    db.setDatabaseName(db_name);
    db.setUserName(db_user);
    db.setPassword(db_pass);
    db.setPort(db_port.toInt());
    bool ok = db.open();

    if (ok)
    {
        qDebug() << "-- database is opened --";
        query = new QSqlQuery(db);
        qDebug() << "LE: " << db.lastError().text();
        if (query->exec("SELECT version()") && query->next())
        {
            qDebug() << query->value(0).toString();
        }
    }
    else
    {
        qDebug() << "database NOT connected";
        qDebug() << db.lastError().text();
    }
}

void Server::launchThread()
{
    fc_thread = new FileConverterThread();
    connect(fc_thread, &FileConverterThread::resultReady, this, &Server::fileConverterResultReady);
    connect(fc_thread, &FileConverterThread::finished, fc_thread, &QObject::deleteLater);
}

void Server::convertFiles()
{
    QString cmd = "SELECT uid, pdf_hex FROM expenses WHERE png_hex IS NULL LIMIT 1";    // query only 1 record
    if (query->exec(cmd))
    {
        if (query->next())  // we have the record
        {
            QString uid = query->value(0).toString();
            QString hex = query->value(1).toString();
            QString filename = tmp_dir+"/"+uid + ".pdf";
            QFile f(filename);
            if (f.open(QIODevice::WriteOnly))
            {
                QByteArray arr = QByteArray::fromHex(hex.toUtf8());
                f.write(arr);
                f.flush();
                f.close();

                QMetaObject::invokeMethod(&converter, "runProcess", Qt::QueuedConnection, Q_ARG(QString, uid), Q_ARG(QString, filename));
            }
        }
    }
}

void Server::pdfFileConverted(QString uid, QString pdf_filename)
{
    fc_thread->start();
    fc_thread->addFileToProcess(uid, pdf_filename);
}

void Server::fileConverterResultReady(QString uid, QString str)
{
    if (!query) return;
    if (query->prepare("UPDATE expenses png_hex VALUES (:png_hex) WHERE uid=:uid"))
    {
        query->bindValue(":uid", uid);
        query->bindValue(":png_hex", str);
        if (!query->exec())
        {
            qDebug() << "Cannot execute inserting PNG HEX: " << query->lastError().text();
        }
    }
}

void Server::setupFilesystemWatch()
{
    bool create_dir = false;
    QDir dir(tmp_dir);
    qDebug() << "Working on : " << dir.absolutePath() << " directory";
    if (!dir.exists())
    {
        create_dir = true;
    }
    else
    {
        QFile f(dir.absolutePath()+"/SERVER.txt");
        if (!f.exists())
        {
            return;
        }

        if (f.open(QIODevice::WriteOnly))
        {
            QTextStream str(&f);
            str << "Temporary file directory for our server application";
            f.close();
        }
    }

    if (create_dir)
    {
        if (!dir.mkpath(tmp_dir))
            return;
        QFile f(dir.absolutePath()+"/SERVER.txt");
        if (f.open(QIODevice::WriteOnly))
        {
            QTextStream str(&f);
            str << "Temporary file directory for our server application";
            f.close();
        }
        else
        {
            qDebug() << "Cannot create SERVER.txt file";
            return;
        }
    }

    QObject::connect(&fswatcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(directoryChanged(const QString&)));
    fswatcher.addPath(tmp_dir);
}

void Server::directoryChangedTimeout()
{
    directoryChanged(tmp_dir);
}

void Server::directoryChanged(const QString& path)
{
    QDir dir(path);
    QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    QDateTime dt = QDateTime::currentDateTime();
    QCryptographicHash cryptohash(QCryptographicHash::Md5);
    int tcnt = 0;
    for (int i = 0; i < files.count(); i++)
    {
        cryptohash.reset();
        QFileInfo fi = files.at(i);
        qDebug() << " ==== CHECKING FILE ==== ";
        qDebug() << " \t absoultePath: " << fi.absolutePath();
        qDebug() << " \t created:      " << fi.birthTime();

        QString finame = fi.baseName();
        QString ext = fi.completeSuffix();
        if (ext.toUpper() == "PNG")
        {
            if (fi.birthTime().secsTo(dt) > 60 * 60)     // File should be at least 1 hour old
            {
                QFile f(fi.absoluteFilePath());
                if (f.open(QIODevice::ReadOnly))
                {
                    QString checksum;
                    if (cryptohash.addData(&f))
                    {
                        checksum = cryptohash.result();
                    }
                    f.close();

                    if (query && query->prepare("UPDATE expenses SET png_md5=:png_md5 WHERE uid=:uid"))
                    {
                        query->bindValue(":png_md5", checksum);
                        query->bindValue(":uid", finame);
                        if (!query->exec())
                            qDebug() << "Cannot save MD5 checksum for " << fi.absoluteFilePath() << " since: " << query->lastError().text();
                        else
                            f.remove();                                // Delete file
                    }
                }
            }
            else
                tcnt++;
        }
    }
    if (tcnt>1)
    {
        QTimer::singleShot(60 * 60 * 1000 + 10 * 1000, this, "directoryChanged"); // triggers 1 hour + 10 secs later
    }
}

