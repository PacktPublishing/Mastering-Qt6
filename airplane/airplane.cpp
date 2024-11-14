#include "airplane.h"

AirPlane::AirPlane(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    statusBar()->hide();
    ui.menuBar->hide();
    ui.gamepage->show();
    showFullScreen();

    QObject::connect(ui.gamepage, SIGNAL(escFromGame()), this, SLOT(showMainMenu()));
    ui.stackedWidget->setCurrentWidget(ui.titlepage);

    //Playing theme music

    player = new QMediaPlayer();
    audioOutput = new QAudioOutput();
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(75);
    audioOutput->setMuted(false);
    showMainMenu();
}

AirPlane::~AirPlane()
{}

