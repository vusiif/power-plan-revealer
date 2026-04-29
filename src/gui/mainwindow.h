#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QHash>
#include <QString>

#include "power_core.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    enum TreeColumn {
        ColumnName = 0,
        ColumnHidden = 1,
        ColumnGuid = 2
    };

    QTreeWidget *tree;
    QLineEdit *searchEdit;
    QPushButton *refreshButton;
    QCheckBox *onlyHiddenCheckBox;
    QLabel *statusLabel;

    QHash<QString, QTreeWidgetItem *> subgroupItems;

    int totalSettingCount;

    void setupUi();
    void reloadTree();
    void addPowerSettingItem(const PowerSettingItem *item);
    void applyFilter();
    void updateStatus();

    static int enumerateCallback(
        const PowerSettingItem *item,
        void *userData
    );
};

#endif
