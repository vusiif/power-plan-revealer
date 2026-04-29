#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

#include "power_core.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QTableWidget *table;
    QPushButton *refreshButton;
    QCheckBox *onlyHiddenCheckBox;
    QLabel *statusLabel;

    void setupUi();
    void reloadTable();
    void addPowerSettingItem(const PowerSettingItem *item);

    static int enumerateCallback(
        const PowerSettingItem *item,
        void *userData
    );
};

#endif
