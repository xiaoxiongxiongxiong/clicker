#include "clicker.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // ∆Ù”√∏ﬂDPIÀı∑≈
    clicker w;
    w.setWindowIcon(QIcon(":/clicker/res/clicker.ico"));
    w.show();
    return a.exec();
}
