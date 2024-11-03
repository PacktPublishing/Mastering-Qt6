#pragma once

#ifndef FILECONVERTER_H
#define FILECONVERTER_H

#include <QThread>
#include <QString>
#include <QMutex>
#include <QWaitCondition>
#include <QMutexLocker>
#include <QQueue>
#include <QPair>
#include <QFile>
#include <QByteArray>

/* The implementation is way sub-optimal! It is only to present the 
usage of the QMutext and QWaitCondition classes. In real application, 
please do not follow this pattern! 
*/

struct ConvInfo
{
public:
    ConvInfo(QString p_uid, QString p_filename) : uid(p_uid), filename(p_filename) {}
    ~ConvInfo() {}
    QString uid;
    QString filename;
};


class FileConverterThread : public QThread
{
Q_OBJECT
public:
    FileConverterThread(QObject *parent=nullptr) : QThread(parent) {}
    ~FileConverterThread() {}
    
    void addFileToProcess(QString uid, QString hex)
    {
        if (uid.isEmpty() || hex.isEmpty()) return;
        param_mutex.lock();
        files.append(ConvInfo(uid, hex));
        param_mutex.unlock();
        waitcondition.wakeAll();
        
    }

    void run() override 
    {
        forever
        {
            run_mutex.lock();
            waitcondition.wait(&run_mutex);
            QString result;

            QString uid;
            QString fname = getFileToProcess(uid);
            QFile f(fname);
            if (f.open(QIODevice::ReadOnly))
            {
                QByteArray ba = f.readAll();
                result = ba.toHex();
                f.close();
            }

            emit resultReady(uid, result);
            run_mutex.unlock();
        }
    }
signals:
    void resultReady(QString uid, QString hex);

private:
    QString getFileToProcess(QString &uid)
    {
        QMutexLocker locker(& param_mutex);
        QString retstr;
        if (files.count())
        {
            ConvInfo ci = files.takeFirst();
            retstr = ci.filename;
            uid = ci.uid;
        }
        return retstr;
    }

private:
    QMutex run_mutex;
    QMutex param_mutex;
    QWaitCondition waitcondition;
    QList<ConvInfo> files;
};

#endif