#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QHash>
#include <QString>

#include "power_core.h"

class QCloseEvent;
class QShowEvent;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void updateTitleBarTheme(bool isDark);
    void readSettings();
    void writeSettings();
    enum TreeColumn {
        ColumnName = 0,
        ColumnHidden = 1,
        ColumnGuid = 2
    };

    enum ItemRole {
        RoleSubgroupGuid = Qt::UserRole,
        RoleSettingGuid = Qt::UserRole + 1,
        RoleHidden = Qt::UserRole + 2,
        RoleItemType = Qt::UserRole + 3,
        RoleDescription = Qt::UserRole + 4,
        RoleAcValue = Qt::UserRole + 5,
        RoleDcValue = Qt::UserRole + 6,
        RoleHasAc = Qt::UserRole + 7,
        RoleHasDc = Qt::UserRole + 8
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
    QPushButton *backupButton;
    QCheckBox *onlyHiddenCheckBox;
    QComboBox *schemeComboBox;
    QPushButton *switchSchemeButton;
    QPushButton *openControlPanelButton;
    QPushButton *presetButton;
    QLabel *activeSchemeLabel;
    QLabel *statusLabel;

    QHash<QString, QTreeWidgetItem *> subgroupItems;

    int totalSettingCount;

    void setupUi();
    void loadStyleSheet(bool isDark);
    void showStartupSafetyNotice();
    void reloadTree();
    void loadSchemes();
    void switchActiveScheme();
    void updateActiveSchemeDisplay();
    void openNativePowerSettings();
    void applyPreset(const QString &presetName, const QStringList &targetGuids);
    void resetToDefaultVisibility();
    void setAllSettingsHidden(bool hidden);
    void addPowerSettingItem(const PowerSettingItem *item);
    void applyFilter();
    void updateActionButtons();

    QTreeWidgetItem *selectedSettingItem() const;

    void setSelectedSettingHidden(bool hidden);
    void copySelectedGuids();
    void copySelectedName();
    void copySelectedRegistryPath();
    void openSelectedInRegedit();
    void backupRegistry();
    void showTreeContextMenu(const QPoint &pos);
    void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column);

    static int enumerateCallback(
        const PowerSettingItem *item,
        void *userData
    );

    static int schemeEnumerateCallback(
        const PowerSchemeItem *item,
        void *userData
    );
};

#endif
