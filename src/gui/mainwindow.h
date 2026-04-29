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

    enum ItemRole {
        RoleSubgroupGuid = Qt::UserRole,
        RoleSettingGuid = Qt::UserRole + 1,
        RoleHidden = Qt::UserRole + 2,
        RoleItemType = Qt::UserRole + 3
    };

    enum ItemType {
        ItemTypeSubgroup = 1,
        ItemTypeSetting = 2
    };

    QTreeWidget *tree;
    QLineEdit *searchEdit;
    QPushButton *refreshButton;
    QPushButton *unhideButton;
    QPushButton *hideButton;
    QPushButton *copyGuidButton;
    QCheckBox *onlyHiddenCheckBox;
    QLabel *statusLabel;

    QHash<QString, QTreeWidgetItem *> subgroupItems;

    int totalSettingCount;

    void setupUi();
    void showStartupSafetyNotice();

    void reloadTree();
    void addPowerSettingItem(const PowerSettingItem *item);
    void applyFilter();
    void updateActionButtons();

    QTreeWidgetItem *selectedSettingItem() const;

    void setSelectedSettingHidden(bool hidden);
    void copySelectedGuids();

    static int enumerateCallback(
        const PowerSettingItem *item,
        void *userData
    );
};

#endif
