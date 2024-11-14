#include "airplane.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AirPlane airplane;
    airplane.show();
    return a.exec();
}
