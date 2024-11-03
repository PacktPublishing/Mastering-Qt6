#pragma once

#ifndef CONVERTER_H
#define CONVERTER_H

/* Codes related to the new learning topics (Chapter 7) implemented in the header file.
* The already known parts (from Chapter 6) are moved into the .cpp.
*/

#include <QObject>
#include <QProcess>

#include <QFile>
#include <QDateTime>
#include <QByteArray>

class Converter : public QObject
{
Q_OBJECT
public:
  Converter(QObject *parent=nullptr) : QObject(parent)
  {
	  process = new QProcess(this);
	  QObject::connect(process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(slot_errorOccurred(QProcess::ProcessError)));
	  QObject::connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slot_finished(int, QProcess::ExitStatus)));
	  QObject::connect(process, SIGNAL(readyReadStandardError()), this, SLOT(slot_readyReadStandardError()));
	  QObject::connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(slot_readyReadStandardOutput()));
	  QObject::connect(process, SIGNAL(started()), this, SLOT(slot_started()));
	  QObject::connect(process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(slot_stateChanged(QProcess::ProcessState)));
  }
  ~Converter() 
  {
	  process->terminate();
  }

signals:
	void pdfFileConverted(QString uid, QString png_filenmae);

protected slots:

	void runProcess(QString p_uid, QString p_filename)
	{
		if (process->state() == QProcess::Running)
			return;
		uid = p_uid;
		QString pdf_filename = p_filename;
		QString png_filename = p_filename.replace(".pdf", ".png");

		QStringList args;
		args << pdf_filename;
		args << png_filename;
		process->startDetached("convert", args);
	}

	void slot_errorOccurred(QProcess::ProcessError error)
	{
		switch (error)
		{
		case QProcess::FailedToStart:
			qDebug() << "PROCESS ERROR: Failed to start";
			break;
		case QProcess::Crashed:
			qDebug() << "PROCESS ERROR: Crashed";
			break;
		case QProcess::Timedout:
			qDebug() << "PROCESS ERROR: Timedout";
			break;
		case QProcess::WriteError:
			qDebug() << "PROCESS ERROR: WriteError";
			break;
		case QProcess::ReadError:
			qDebug() << "PROCESS ERROR: ReadError";
			break;
		case QProcess::UnknownError:
			qDebug() << "PROCESS ERROR: UnknownError";
			break;
		}
	}

	void slot_finished(int exitCode, QProcess::ExitStatus exitStatus)
	{
		QString statusstr;
		switch (exitStatus)
		{
			case QProcess::NormalExit:
				statusstr = "Normal";		// save the generated PNG into the database
				{
					emit pdfFileConverted(uid, png_filename);
				}
				break;
			case QProcess::CrashExit:
				statusstr = "Crashed";
				break;
		}
		qDebug() << "FINISHED: " + QString::number(exitCode) + " STATUS : " + statusstr;
	}

	void slot_readyReadStandardError()
	{
		QByteArray arr = process->readAllStandardError();
		QString str = QString(arr);
		QStringList lst = str.split("\n");
		for (int i = 0; i < lst.count(); i++)
		{
			qDebug() << lst.at(i);
		}
	}

	void slot_readyReadStandardOutput()
	{
		QByteArray arr = process->readAllStandardOutput();
		QString str = QString(arr);
		QStringList lst = str.split("\n");
		for (int i = 0; i < lst.count(); i++)
		{
			qDebug() << lst.at(i);
		}
	}

	void slot_started()
	{
		qDebug() << "process started";
	}

	void slot_stateChanged(QProcess::ProcessState newState)
	{
		QString statestr;
		switch (newState)
		{
		case QProcess::NotRunning:
			statestr = "Not running";
			break;
		case QProcess::Starting:
			statestr = "Starting";
			break;
		case QProcess::Running:
			statestr = "Running";
			break;
		}
		qDebug() << "PROC STATE CHANGED: "+statestr;
	}

private:
    QProcess* process;
	qint64 pid;

	QString     uid;			// The current uid we are working with
	QString     pdf_filename;	// Path of the input pdf file
	QString     png_filename;	// Path of the output png file

};

#endif