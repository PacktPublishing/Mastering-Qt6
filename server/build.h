#ifndef BUILD_H
#define BUILD_H

#include <QtWidgets/QMainWindow>
#include <QProcess>
#include <QDebug>

#include "ui_build.h"
#include "buildengine.h"
#include "repository.h"
#include "updater.h"

enum CompileStates
{
	UNSET		= 0,
	ERROR		= 1,
	PROGRESS	= 2,
	OK			= 4,
	DISABLED	= 8
};

enum CompileJobs	// and index too
{
	PullSource		= 0,
	TimeStamping	= 1,
	PushSource		= 2,
	Compile			= 3,
	PackInstall		= 4,
	DeployInstall	= 5,
	PackDelivery	= 6,
	DeployDelivery	= 7
};

class build : public QMainWindow
{
	Q_OBJECT

public:
	build(QWidget *parent = 0);
	~build();

protected:
	void generateProcessCommand(int stageidx, QStringList &dir, QStringList &command);
	
signals:
	void stageCompleted(int stageidx, int retval);
	void signal_runProcess();

protected slots:
	void on_compilenow_clicked();
	void on_pullsource_clicked();
	void on_timestamping_clicked();
	void on_pushsource_clicked();
	void on_compile_clicked();
	void on_packinstall_clicked();
	void on_deployinstall_clicked();
	void on_packdelivery_clicked();
	void on_deploydelivery_clicked();
	void on_test_clicked();

	void cancelAllJobs();
	void runCycle();
	void deployInstall();
	void deployDelivery();
	void runStage(int stageidx);
	void setButtonState(int buttindex, int state);
	void setButtonState(QPushButton *butt, int state);
	void updateButtonStates();
	void slot_stageCompleted(int stageidx, int retval);
	void compileLog(QString str);

	void runProcess();

	void slot_errorOccurred(QProcess::ProcessError error);
	void slot_finished(int exitCode, QProcess::ExitStatus exitStatus);
	void slot_readyReadStandardError();
	void slot_readyReadStandardOutput();
	void slot_started();
	void slot_stateChanged(QProcess::ProcessState newState);

private:
	Ui::buildClass ui;
	BuildEngine engine;
	QProcess *process;

	int currstage;
	QList<int> states;
	QList<QPushButton *> butts;

	QStringList dirs;
	QStringList cmds;
	

};

#endif // BUILD_H
