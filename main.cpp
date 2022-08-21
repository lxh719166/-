#include "mainwindow.h"
#include <QApplication>
#include "login.h"
#include <QTextStream>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    login log_window;
    log_window.show();
    MainWindow w;
    QObject::connect(&log_window,&login::showmain,&w,&MainWindow::receiveLogin);
    //w.show();
    return a.exec();
}
