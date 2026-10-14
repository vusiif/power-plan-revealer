#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("PowerPlanRevealer");
    QCoreApplication::setApplicationName("PowerPlanRevealer");

    MainWindow window;
    window.show();

    return QApplication::exec();
}
