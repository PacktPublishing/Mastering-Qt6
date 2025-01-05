#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AirPlane.h"
#include "gamewidget.h"

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

class AirPlane : public QMainWindow
{
    Q_OBJECT

public:
    AirPlane(QWidget *parent = nullptr);
    ~AirPlane();

protected slots:
    void on_button_play_clicked()
    {
        ui.stackedWidget->setCurrentWidget(ui.gamepage);
        int rndi = rand() % 3 + 1;
        player->setSource(QUrl("qrc:/AirPlane/music/level"+QString::number(rndi)+".wav"));
        player->setLoops(999);
        player->play();

        ui.gamepage->startGame();
    }
    void on_button_about_clicked()
    {
        player->setSource(QUrl("qrc:/AirPlane/music/credit.wav"));
        player->setLoops(999);
        player->play();
        ui.stackedWidget->setCurrentWidget(ui.aboutpage);
    }
    void on_button_exit_clicked()
    {
        qApp->exit();
    }

    void on_button_about_back_clicked()
    {
        showMainMenu();
    }

    void showMainMenu()
    {
        ui.stackedWidget->setCurrentIndex(0);
        player->setSource(QUrl("qrc:/AirPlane/music/titlescreen.wav"));
        player->setLoops(999);
        player->play();
        ui.stackedWidget->show();
    }

private:
    Ui::AirPlaneClass ui;
    QMediaPlayer* player;
    QAudioOutput* audioOutput;

};
