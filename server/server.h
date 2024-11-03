#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QTcpServer>
#include <QHostAddress>
#include <QHash>
#include <QTcpSocket>
#include <QTimer>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QFileSystemWatcher>
#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>

#include "statistics.h"
#include "converter.h"
#include "fileconverter.h"

class Server : public QObject
{
    // Q_OBJECT macro is always needed to be defined when class inherits from QObject
    Q_OBJECT
public:
    Server(QObject* parent = nullptr) : QObject(parent), query(nullptr)
    {

        // Creating TCP server which handles all incoming connections
        setupTCPServer();
        setupDatabase();
        launchThread();

        // Creating processes and threads for file conversions
        tmp_dir = "tmpDir";         // directory where the temporary files are stored

        timer.setSingleShot(false);
        // timer.start(5 * 60 * 1000); // runs in every 5 minutes
        QMetaObject::invokeMethod(this, "convertFiles", Qt::QueuedConnection);
        QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(convertFiles()));

        QObject::connect(&converter, SIGNAL(pdfFileConverted(QString)), this, SLOT(pdfFileConverted(QString)));
        connect(fc_thread, &FileConverterThread::resultReady, this, &Server::fileConverterResultReady);

        setupFilesystemWatch();
    }

    ~Server()
    {}

protected:
    void setupDatabase();
    void setupFilesystemWatch();

protected slots:
    void setupTCPServer();
    void handleNewConnection();
    void handleNewDataFromSocket();
    void handleSocketClosed();
    void releaseAllConnections();
    void convertFiles();
    void launchThread();
    void directoryChanged(const QString& path);
    void directoryChangedTimeout();

public slots:
    void pdfFileConverted(QString uid, QString png_filename);
    void fileConverterResultReady(QString uid, QString png);

private:
    // Handling all client connections
    QTcpServer* tcpserver = nullptr;

    // Incremental index for the sockets identifications
    int client_index = 0;

    // Store all active connections here
    QHash<int, QTcpSocket*> clients;

    QString tmp_dir;
    QTimer timer;
    QSqlDatabase db;
    QSqlQuery* query;
    Converter converter;
    FileConverterThread* fc_thread;

    QFileSystemWatcher fswatcher;
    
};

#endif