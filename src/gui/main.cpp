#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QTranslator>
#include <QLocale>
#include <QSettings>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(":/app.ico"));
    QCoreApplication::setOrganizationName("PowerPlanRevealer");
    QCoreApplication::setApplicationName("PowerPlanRevealer");

    QSettings settings("PowerPlanRevealer", "PowerPlanRevealer");
    QString language = settings.value("general/language", "auto").toString();

    QTranslator translator;
    bool loaded = false;

    if (language == "zh_CN") {
        loaded = translator.load(":/translations/power-plan-revealer_zh_CN.qm");
    } else if (language == "en") {
        loaded = translator.load(":/translations/power-plan-revealer_en.qm");
    } else {
        // Auto-detect system locale
        QLocale systemLocale = QLocale::system();
        if (systemLocale.language() == QLocale::Chinese) {
            loaded = translator.load(":/translations/power-plan-revealer_zh_CN.qm");
        } else {
            loaded = translator.load(":/translations/power-plan-revealer_en.qm");
        }
    }

    if (loaded) {
        app.installTranslator(&translator);
    }

    MainWindow window;
    window.show();

    return QApplication::exec();
}
