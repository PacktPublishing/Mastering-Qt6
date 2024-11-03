#include "budget.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    budget w;
    w.show();
    return a.exec();
}
