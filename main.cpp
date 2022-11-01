#include "mainwindow.h"
#include "initialization.h"
#include <QApplication>
#include <QTextCodec>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8")); //изменения
    initialization w;
    w.show();
    return a.exec();
}
