#include "build.h"

build::build(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	process = new QProcess(this);

	QObject::connect(process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(slot_errorOccurred(QProcess::ProcessError)));
	QObject::connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slot_finished(int, QProcess::ExitStatus)));
	QObject::connect(process, SIGNAL(readyReadStandardError()), this, SLOT(slot_readyReadStandardError()));
	QObject::connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(slot_readyReadStandardOutput()));
	QObject::connect(process, SIGNAL(started()), this, SLOT(slot_started()));
	QObject::connect(process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(slot_stateChanged(QProcess::ProcessState)));
	
	butts.append(ui.pullsource);	// should be in compileJobs order!
	butts.append(ui.timestamping);
	butts.append(ui.pushsource);
	butts.append(ui.compile);
	butts.append(ui.packinstall);
	butts.append(ui.deployinstall);
	butts.append(ui.packdelivery);
	butts.append(ui.deploydelivery);

	for (int i = 0; i < butts.count(); i++)
	{
		states.append(UNSET);
	}
	updateButtonStates();

	QObject::connect(this, SIGNAL(stageCompleted(int, int)), this, SLOT(slot_stageCompleted(int, int)), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(signal_runProcess()), this, SLOT(runProcess()), Qt::QueuedConnection);
}

build::~build()
{
}
void build::cancelAllJobs()
{
}

void build::setButtonState(int buttindex, int state)
{
	setButtonState(butts.at(buttindex), state);
	states[buttindex] = state;
	updateButtonStates();
}

void build::setButtonState(QPushButton *butt, int state)
{
	if (!butt)return;

	switch(state)
	{
		case UNSET:
			butt->setStyleSheet("background-color: rgb(128,128,128);");
			break;
		case ERROR:
			butt->setStyleSheet("background-color: rgb(255, 30, 30);");
			break;
		case PROGRESS:
			butt->setStyleSheet("background-color: rgb(255, 255, 30);");
			break;
		case OK:
			butt->setStyleSheet("background-color: rgb(30, 255, 30);");
			break;
	}
}

void build::updateButtonStates()
{
	for (int i = 0; i < butts.count(); i++)
	{
		setButtonState(butts.at(i), states.at(i));
	}
}

void build::on_test_clicked()
{
	UpdateManager *upd = new UpdateManager(this);
}

void build::on_compilenow_clicked()
{
	setButtonState(ui.errorlamp, UNSET);
	setButtonState(PullSource, PROGRESS);
	setButtonState(TimeStamping, PROGRESS);
	setButtonState(PushSource, PROGRESS);
	setButtonState(Compile, PROGRESS);
	setButtonState(PackInstall, PROGRESS);
	setButtonState(DeployInstall, PROGRESS);
	setButtonState(PackDelivery, PROGRESS);
	setButtonState(DeployDelivery, PROGRESS);

	runCycle();
}

void build::on_pullsource_clicked()
{
}

void build::on_pushsource_clicked()
{
}


void build::on_timestamping_clicked()
{
}

void build::on_compile_clicked()
{
}

void build::on_packinstall_clicked()
{
}

void build::on_deployinstall_clicked()
{
}

void build::on_packdelivery_clicked()
{
}

void build::on_deploydelivery_clicked()
{
}


/* ====================================================================== */

void build::runCycle()
{
	currstage = -1;
	emit stageCompleted(currstage, 0);
}

void build::slot_stageCompleted(int stageidx, int retval)
{
	if (retval != 0)
	{
		setButtonState(stageidx, ERROR);
		return;
	}
	else
	{
		if (currstage != -1)
		{
			setButtonState(currstage, OK);
		}

		int i;
		for (i= currstage+1;i<butts.count() && states.at(i)!=PROGRESS; i++)
		{
		}
		currstage = i;
			
		if (currstage < butts.count())
		{
			runStage(currstage);
		}
	}
}

void build::runStage(int stageidx)
{
	bool pack = true;
	QString cmd;
	QString dir;
	switch (stageidx)
	{
		case PullSource:
			emit stageCompleted(PullSource, 0);
			break;
		case TimeStamping:
			engine.updateAllRCs();
			{
				compileLog("==========================================================================");
				compileLog(" TIMESTAMP: " + engine.getTimeStamp());
				compileLog("==========================================================================");
				ui.compiletimestamp->setText(engine.getTimeStamp());
			}
			emit stageCompleted(TimeStamping, 0);
			break;
		case PushSource:
			emit stageCompleted(PushSource, 0);
			break;
		case Compile:
			generateProcessCommand(Compile, dirs, cmds);
			emit signal_runProcess();
			//emit stageCompleted(PushSource, 0);
			break;
		case PackInstall:
			generateProcessCommand(PackInstall, dirs, cmds);
			emit signal_runProcess();
			//emit stageCompleted(PackInstall, 0);
			break;
		case DeployInstall:
			deployInstall();
			emit stageCompleted(DeployInstall, 0);
			break;
		case PackDelivery:
			if (pack)
			{
				generateProcessCommand(PackDelivery, dirs, cmds);
				emit signal_runProcess();
			}
			else
			{
				emit stageCompleted(PackDelivery, 0);
			}
			break;
		case DeployDelivery:
			deployDelivery();
			emit stageCompleted(DeployDelivery, 0);
			break;
	}
}

void build::generateProcessCommand(int stageidx, QStringList &dir, QStringList &cmd)
{
	dir.clear();
	cmd.clear();
	QString ts = engine.getTimeStamp();
	switch (stageidx)
	{
		case PullSource:
			break;
		case TimeStamping:
			break;
		case PushSource:
			break;
		case Compile:
			dir << "c:\\Program Files (x86)\\MSBuild\\14.0\\Bin";
			// cmd << "msbuild d:\\isoteq\\isoteq.sln /t:Rebuild /p:Configuration=Release /m:8";
			cmd << "msbuild d:\\isoteq\\isoteq.sln /p:Configuration=Release /m:4";
			break;
		case PackInstall:
			dir << "c:\\Program Files (x86)\\Inno Setup 5";
			cmd << "iscc /Qp d:\\ISOTEQ\\innoscript\\isoteq_designer.iss";
//			dir << "c:\\Program Files (x86)\\Inno Setup 5";
//			cmd << "iscc /Qq d:\\ISOTEQ\\innoscript\\isoteq_designer_light.iss";
//			dir << "c:\\Program Files (x86)\\Inno Setup 5";
//			cmd << "iscc /Qp d:\\ISOTEQ\\innoscript\\isoteq_manager.iss";
			break;
		case DeployInstall:
			break;
		case PackDelivery:
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_"+  ts.replace(".","_")  + ".zip d:\\isoteq\\*.cpp";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.c";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.h";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.hpp";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.pdb";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.ui";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.exe";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.dll";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.pro";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.qrc";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.png";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.svg";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.gif";
			cmd << "7z.exe a -r s:\\deploy\\delivery\\isoteq_" + ts.replace(".", "_") + ".zip d:\\isoteq\\*.ico";
			for (int i = 0; i < cmd.count(); i++) dir << "c:\\Program Files\\7-Zip";
			break;
		case DeployDelivery:
			break;
	}
}

void build::deployInstall()
{}

void build::deployDelivery()
{
	QString datestr = engine.getDateString();
	RepositoryManager rep;
	QObject::connect(&rep, SIGNAL(signal_log(QString)), this, SLOT(compileLog(QString)));
	rep.setDateString(datestr);
	rep.makeRelease("d:\\isoteq\\innoscript\\isoteq_designer.iss", "d:\\isoteq_datadir", "i:\\repository");
//	rep.makeRelease("d:\\isoteq\\innoscript\\isoteq_manager.iss", "d:\\isoteq_datadir", "i:\\repository");
}
/*================================================== PROCESS HANDLING ================================*/

void build::compileLog(QString str)
{
	str = str.simplified();
	if (str.isEmpty()) return;
	QDateTime dt = QDateTime::currentDateTime();
	QString l = dt.toString("hh:mm:ss.zzz") + " " + str;
	ui.compilelog->append(l);
}

void build::runProcess()
{
	if (cmds.count() == 0)
	{
		emit stageCompleted(currstage, 0);
		return;
	}
	QString cmd = cmds.takeFirst();
	QString dir = dirs.takeFirst();
	process->setWorkingDirectory(dir);
	process->start("cmd.exe /C "+cmd);
	process->waitForStarted();
}

void build::slot_errorOccurred(QProcess::ProcessError error)
{
	switch (error)
	{
		case QProcess::FailedToStart:
			compileLog("PROCESS ERROR: Failed to start");
			break;
		case QProcess::Crashed:
			compileLog("PROCESS ERROR: Crashed");
			break;
		case QProcess::Timedout:
			compileLog("PROCESS ERROR: Timedout");
			break;
		case QProcess::WriteError:
			compileLog("PROCESS ERROR: WriteError");
			break;
		case QProcess::ReadError:
			compileLog("PROCESS ERROR: ReadError");
			break;
		case QProcess::UnknownError:
			compileLog("PROCESS ERROR: UnknownError");
			break;
	}
}

void build::slot_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QString statusstr;
	switch (exitStatus)
	{
	case QProcess::NormalExit:
		statusstr = "Normal";
		break;
	case QProcess::CrashExit:
		statusstr = "Crashed";
		break;
	}
	compileLog("FINISHED: " + QString::number(exitCode) + " STATUS: " + statusstr);
	if (exitCode == 0 && exitStatus == QProcess::NormalExit)
	{
		emit signal_runProcess();
	}
	else
	{
		setButtonState(ui.errorlamp, ERROR);
	}
}

void build::slot_readyReadStandardError()
{
	QByteArray arr = process->readAllStandardError();
	QString str = QString(arr);
	QStringList lst = str.split("\n");
	for (int i = 0; i < lst.count(); i++)
	{
		compileLog(lst.at(i));
	}
}

void build::slot_readyReadStandardOutput()
{
	QByteArray arr = process->readAllStandardOutput();
	QString str = QString(arr);
	QStringList lst = str.split("\n");
	for (int i = 0; i < lst.count(); i++)
	{
		compileLog(lst.at(i));
	}
}

void build::slot_started()
{
	compileLog("process started");
}

void build::slot_stateChanged(QProcess::ProcessState newState)
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
	//compileLog("PROC STATE CHANGED: "+statestr);
}

