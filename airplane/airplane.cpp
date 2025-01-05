#include "airplane.h"

AirPlane::AirPlane(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    QObject::connect(ui.button_play, SIGNAL(clicked()), this, SLOT(on_button_play_clicked()));
    QObject::connect(ui.gamepage, SIGNAL(escFromGame()), this, SLOT(showMainMenu()));
    
    //Playing theme music

    player = new QMediaPlayer();
    audioOutput = new QAudioOutput();
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(35);
    audioOutput->setMuted(false);
    showMainMenu();
}

AirPlane::~AirPlane()
{}

