#include "scopymainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ScopyMainWindow w;
    w.show();
    return a.exec();
}
