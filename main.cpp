#include "checkdata.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CheckData w;
    w.show();

    return a.exec();
}
