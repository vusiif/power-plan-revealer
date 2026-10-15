#include "mainwindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(":/app.ico"));
    QCoreApplication::setOrganizationName("PowerPlanRevealer");
    QCoreApplication::setApplicationName("PowerPlanRevealer");

    MainWindow window;
    window.show();

    return QApplication::exec();
}
