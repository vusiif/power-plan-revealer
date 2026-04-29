#include "mainwindow.h"

#include "power_attr.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QString>
#include <QTreeWidgetItem>
#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QSettings>
#include <QFont>

#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      tree(nullptr),
      searchEdit(nullptr),
      refreshButton(nullptr),
      unhideButton(nullptr),
      hideButton(nullptr),
      copyGuidButton(nullptr),
      onlyHiddenCheckBox(nullptr),
      statusLabel(nullptr),
      totalSettingCount(0) {
    setupUi();
    showStartupSafetyNotice();
    reloadTree();
}

void MainWindow::setupUi() {
    setWindowTitle("PowerPlanRevealer");

    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    auto *topLayout = new QHBoxLayout();

    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Search name or GUID...");
    searchEdit->setClearButtonEnabled(true);

    refreshButton = new QPushButton("Refresh", this);
    unhideButton = new QPushButton("Unhide Selected", this);
    hideButton = new QPushButton("Hide Selected", this);
    copyGuidButton = new QPushButton("Copy GUIDs", this);

    unhideButton->setEnabled(false);
    hideButton->setEnabled(false);
    copyGuidButton->setEnabled(false);

    onlyHiddenCheckBox = new QCheckBox("Only hidden", this);
    onlyHiddenCheckBox->setChecked(true);

    statusLabel = new QLabel(this);

    topLayout->addWidget(searchEdit, 1);
    topLayout->addWidget(refreshButton);
    topLayout->addWidget(unhideButton);
    topLayout->addWidget(hideButton);
    topLayout->addWidget(copyGuidButton);
    topLayout->addWidget(onlyHiddenCheckBox);
    topLayout->addWidget(statusLabel);

    tree = new QTreeWidget(this);
    tree->setColumnCount(3);
    tree->setHeaderLabels({
        "Name",
        "Hidden",
        "GUID"
    });

    tree->setRootIsDecorated(true);
    tree->setItemsExpandable(true);
    tree->setExpandsOnDoubleClick(true);
    tree->setAlternatingRowColors(true);
    tree->setUniformRowHeights(true);
    tree->setIndentation(22);

    tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    tree->setSelectionMode(QAbstractItemView::SingleSelection);
    tree->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tree->header()->setStretchLastSection(false);
    tree->header()->setSectionResizeMode(ColumnName, QHeaderView::Stretch);
    tree->header()->setSectionResizeMode(ColumnHidden, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(ColumnGuid, QHeaderView::ResizeToContents);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(tree);

    setCentralWidget(central);

    setStyleSheet(R"(
        QMainWindow {
            background: #f6f7f9;
        }

        QLineEdit {
            padding: 6px 10px;
            border: 1px solid #cfd4dc;
            border-radius: 6px;
            background: white;
        }

        QPushButton {
            padding: 6px 12px;
            border: 1px solid #c5cbd3;
            border-radius: 6px;
            background: #ffffff;
        }

        QPushButton:hover {
            background: #eef3ff;
        }

        QPushButton:disabled {
            color: #9ca3af;
            background: #f3f4f6;
        }

        QCheckBox {
            spacing: 6px;
        }

        QTreeWidget {
            background: white;
            border: 1px solid #d8dde5;
            border-radius: 8px;
            alternate-background-color: #f8fafc;
        }

        QTreeWidget::item {
            padding: 4px;
        }

        QTreeWidget::item:selected {
            background: #dbeafe;
            color: #111827;
        }

        QHeaderView::section {
            padding: 6px;
            background: #eef1f5;
            border: none;
            border-right: 1px solid #d8dde5;
            font-weight: 600;
        }

        QLabel {
            color: #4b5563;
        }
    )");

    connect(
        refreshButton,
        &QPushButton::clicked,
        this,
        &MainWindow::reloadTree
    );

    connect(
        unhideButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setSelectedSettingHidden(false);
        }
    );

    connect(
        hideButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setSelectedSettingHidden(true);
        }
    );

    connect(
        copyGuidButton,
        &QPushButton::clicked,
        this,
        &MainWindow::copySelectedGuids
    );

    connect(
        onlyHiddenCheckBox,
        &QCheckBox::checkStateChanged,
        this,
        [this](Qt::CheckState) {
            reloadTree();
        }
    );

    connect(
        searchEdit,
        &QLineEdit::textChanged,
        this,
        [this]() {
            applyFilter();
        }
    );

    connect(
        tree,
        &QTreeWidget::currentItemChanged,
        this,
        [this](QTreeWidgetItem *, QTreeWidgetItem *) {
            updateActionButtons();
        }
    );
}

void MainWindow::showStartupSafetyNotice() {
    QSettings settings(
        "PowerPlanRevealer",
        "PowerPlanRevealer"
    );

    const bool skipNotice = settings.value(
        "safety/skipStartupNotice",
        false
    ).toBool();

    if (skipNotice) {
        return;
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle("Safety Notice");

    box.setText(
        "PowerPlanRevealer can modify Windows power setting visibility."
    );

    box.setInformativeText(
        "Before using Hide or Unhide, it is recommended to create a system restore point "
        "or export the related registry keys.\n\n"
        "Recommended registry path to back up:\n"
        "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerSettings\n\n"
        "This tool only changes the hide attribute of selected power settings, "
        "but these are still system-level settings. Please keep a backup before making changes."
    );

    box.setStandardButtons(QMessageBox::Ok);
    box.setDefaultButton(QMessageBox::Ok);

    auto *dontShowAgain = new QCheckBox(
        "Don't show this notice again",
        &box
    );

    box.setCheckBox(dontShowAgain);

    box.exec();

    if (dontShowAgain->isChecked()) {
        settings.setValue("safety/skipStartupNotice", true);
    }
}

void MainWindow::reloadTree() {
    tree->setUpdatesEnabled(false);
    tree->clear();
    subgroupItems.clear();
    totalSettingCount = 0;

    updateActionButtons();

    const int hiddenOnly = onlyHiddenCheckBox->isChecked() ? 1 : 0;

    int rc = enumerate_power_settings(
        hiddenOnly,
        MainWindow::enumerateCallback,
        this
    );

    if (rc != ERROR_SUCCESS) {
        tree->setUpdatesEnabled(true);

        QMessageBox::critical(
            this,
            "Error",
            QString("Failed to enumerate power settings. Error code: %1").arg(rc)
        );

        statusLabel->setText("Failed");
        return;
    }

    tree->sortItems(ColumnName, Qt::AscendingOrder);

    if (onlyHiddenCheckBox->isChecked()) {
        tree->expandAll();
    } else {
        tree->collapseAll();
    }

    applyFilter();
    updateActionButtons();

    tree->setUpdatesEnabled(true);
}

int MainWindow::enumerateCallback(
    const PowerSettingItem *item,
    void *userData
) {
    auto *window = static_cast<MainWindow *>(userData);

    if (window != nullptr) {
        window->addPowerSettingItem(item);
    }

    return 0;
}

void MainWindow::addPowerSettingItem(const PowerSettingItem *item) {
    if (item == nullptr) {
        return;
    }

    const QString subgroupName = QString::fromWCharArray(item->subgroup_name);
    const QString settingName = QString::fromWCharArray(item->setting_name);
    const QString subgroupGuid = QString::fromWCharArray(item->subgroup_guid_text);
    const QString settingGuid = QString::fromWCharArray(item->setting_guid_text);
    const QString hiddenText = item->hidden ? "Yes" : "No";

    QTreeWidgetItem *subgroupItem = subgroupItems.value(subgroupGuid, nullptr);

    if (subgroupItem == nullptr) {
        subgroupItem = new QTreeWidgetItem(tree);

        subgroupItem->setText(ColumnName, subgroupName);
        subgroupItem->setText(ColumnHidden, "");
        subgroupItem->setText(ColumnGuid, subgroupGuid);

        subgroupItem->setData(ColumnName, RoleSubgroupGuid, subgroupGuid);
        subgroupItem->setData(ColumnName, RoleSettingGuid, "");
        subgroupItem->setData(ColumnName, RoleHidden, 0);
        subgroupItem->setData(ColumnName, RoleItemType, ItemTypeSubgroup);

        subgroupItem->setToolTip(ColumnName, subgroupName);
        subgroupItem->setToolTip(ColumnGuid, subgroupGuid);

        QFont font = subgroupItem->font(ColumnName);
        font.setBold(true);
        subgroupItem->setFont(ColumnName, font);

        subgroupItems.insert(subgroupGuid, subgroupItem);
    }

    auto *settingItem = new QTreeWidgetItem(subgroupItem);

    settingItem->setText(ColumnName, settingName);
    settingItem->setText(ColumnHidden, hiddenText);
    settingItem->setText(ColumnGuid, settingGuid);

    settingItem->setData(ColumnName, RoleSubgroupGuid, subgroupGuid);
    settingItem->setData(ColumnName, RoleSettingGuid, settingGuid);
    settingItem->setData(ColumnName, RoleHidden, item->hidden ? 1 : 0);
    settingItem->setData(ColumnName, RoleItemType, ItemTypeSetting);

    settingItem->setToolTip(ColumnName, settingName);
    settingItem->setToolTip(ColumnGuid, settingGuid);

    totalSettingCount++;
}

void MainWindow::applyFilter() {
    const QString query = searchEdit->text().trimmed();
    const bool hasQuery = !query.isEmpty();

    int visibleSubgroupCount = 0;
    int visibleSettingCount = 0;

    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *subgroupItem = tree->topLevelItem(i);

        const bool subgroupMatches =
            subgroupItem->text(ColumnName).contains(query, Qt::CaseInsensitive) ||
            subgroupItem->text(ColumnGuid).contains(query, Qt::CaseInsensitive);

        bool hasVisibleChild = false;

        for (int j = 0; j < subgroupItem->childCount(); ++j) {
            QTreeWidgetItem *settingItem = subgroupItem->child(j);

            const bool settingMatches =
                settingItem->text(ColumnName).contains(query, Qt::CaseInsensitive) ||
                settingItem->text(ColumnHidden).contains(query, Qt::CaseInsensitive) ||
                settingItem->text(ColumnGuid).contains(query, Qt::CaseInsensitive);

            const bool visible = !hasQuery || subgroupMatches || settingMatches;

            settingItem->setHidden(!visible);

            if (visible) {
                hasVisibleChild = true;
                visibleSettingCount++;
            }
        }

        const bool subgroupVisible = !hasQuery || subgroupMatches || hasVisibleChild;

        subgroupItem->setHidden(!subgroupVisible);

        if (subgroupVisible) {
            visibleSubgroupCount++;
        }

        if (hasQuery && subgroupVisible) {
            subgroupItem->setExpanded(true);
        }
    }

    if (hasQuery) {
        statusLabel->setText(
            QString("%1 / %2 item(s), %3 subgroup(s)")
                .arg(visibleSettingCount)
                .arg(totalSettingCount)
                .arg(visibleSubgroupCount)
        );
    } else {
        statusLabel->setText(
            QString("%1 item(s), %2 subgroup(s)")
                .arg(totalSettingCount)
                .arg(tree->topLevelItemCount())
        );
    }

    updateActionButtons();
}

QTreeWidgetItem *MainWindow::selectedSettingItem() const {
    QTreeWidgetItem *item = tree->currentItem();

    if (item == nullptr) {
        return nullptr;
    }

    const int itemType = item->data(ColumnName, RoleItemType).toInt();

    if (itemType != ItemTypeSetting) {
        return nullptr;
    }

    return item;
}

void MainWindow::updateActionButtons() {
    QTreeWidgetItem *item = selectedSettingItem();

    if (item == nullptr) {
        unhideButton->setEnabled(false);
        hideButton->setEnabled(false);
        copyGuidButton->setEnabled(false);
        return;
    }

    const bool hidden = item->data(ColumnName, RoleHidden).toInt() != 0;

    unhideButton->setEnabled(hidden);
    hideButton->setEnabled(!hidden);
    copyGuidButton->setEnabled(true);
}

void MainWindow::setSelectedSettingHidden(bool hidden) {
    QTreeWidgetItem *item = selectedSettingItem();

    if (item == nullptr) {
        QMessageBox::information(
            this,
            "No setting selected",
            "Please select a power setting item first."
        );
        return;
    }

    const bool currentlyHidden = item->data(ColumnName, RoleHidden).toInt() != 0;

    if (currentlyHidden == hidden) {
        QMessageBox::information(
            this,
            "No change needed",
            hidden
                ? "This setting is already hidden."
                : "This setting is already visible."
        );
        return;
    }

    const QString settingName = item->text(ColumnName);
    const QString subgroupGuidText = item->data(ColumnName, RoleSubgroupGuid).toString();
    const QString settingGuidText = item->data(ColumnName, RoleSettingGuid).toString();

    const QString currentStateText = currentlyHidden ? "Hidden" : "Visible";
    const QString targetStateText = hidden ? "Hidden" : "Visible";

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        hidden ? "Confirm Hide Setting" : "Confirm Unhide Setting",
        QString(
            "Please confirm this change:\n\n"
            "Setting:\n%1\n\n"
            "Current state: %2\n"
            "Target state : %3\n\n"
            "Subgroup GUID:\n%4\n\n"
            "Setting GUID:\n%5\n\n"
            "It is recommended to back up the registry or create a restore point "
            "before modifying system power settings."
        ).arg(
            settingName,
            currentStateText,
            targetStateText,
            subgroupGuidText,
            settingGuidText
        ),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (answer != QMessageBox::Yes) {
        return;
    }

    GUID subgroupGuid;
    GUID settingGuid;

    const std::wstring subgroupWide = subgroupGuidText.toStdWString();
    const std::wstring settingWide = settingGuidText.toStdWString();

    if (!parse_guid_text(subgroupWide.c_str(), &subgroupGuid)) {
        QMessageBox::critical(
            this,
            "Invalid GUID",
            QString("Invalid subgroup GUID:\n%1").arg(subgroupGuidText)
        );
        return;
    }

    if (!parse_guid_text(settingWide.c_str(), &settingGuid)) {
        QMessageBox::critical(
            this,
            "Invalid GUID",
            QString("Invalid setting GUID:\n%1").arg(settingGuidText)
        );
        return;
    }

    const DWORD beforeAttributes = get_setting_attributes(
        &subgroupGuid,
        &settingGuid
    );

    const DWORD rc = hidden
        ? hide_setting(&subgroupGuid, &settingGuid)
        : unhide_setting(&subgroupGuid, &settingGuid);

    const DWORD afterAttributes = get_setting_attributes(
        &subgroupGuid,
        &settingGuid
    );

    if (rc != ERROR_SUCCESS) {
        QString message = QString("Operation failed. Error code: %1").arg(rc);

        if (rc == ERROR_ACCESS_DENIED) {
            message += "\n\nAccess denied. Please make sure the GUI is running as administrator.";
        }

        QMessageBox::critical(
            this,
            "Operation failed",
            message
        );

        return;
    }

    const DWORD refreshRc = refresh_active_scheme();

    if (refreshRc != ERROR_SUCCESS) {
        QMessageBox::warning(
            this,
            "Refresh active scheme failed",
            QString("The setting was changed, but refreshing the active power scheme failed. Error code: %1")
                .arg(refreshRc)
        );
    }

    QMessageBox::information(
        this,
        "Done",
        QString(
            "%1\n\n"
            "Setting:\n%2\n\n"
            "Before attributes: 0x%3\n"
            "After attributes : 0x%4"
        ).arg(
            hidden ? "The setting has been hidden." : "The setting has been unhidden.",
            settingName,
            QString::number(beforeAttributes, 16).toUpper().rightJustified(8, '0'),
            QString::number(afterAttributes, 16).toUpper().rightJustified(8, '0')
        )
    );

    statusLabel->setText(
        hidden ? "Setting hidden" : "Setting unhidden"
    );

    reloadTree();
}

void MainWindow::copySelectedGuids() {
    QTreeWidgetItem *item = selectedSettingItem();

    if (item == nullptr) {
        QMessageBox::information(
            this,
            "No setting selected",
            "Please select a power setting item first."
        );
        return;
    }

    const QString subgroupGuidText = item->data(ColumnName, RoleSubgroupGuid).toString();
    const QString settingGuidText = item->data(ColumnName, RoleSettingGuid).toString();

    const QString text = QString("Subgroup GUID: %1\nSetting GUID: %2")
        .arg(subgroupGuidText, settingGuidText);

    QApplication::clipboard()->setText(text);

    statusLabel->setText("GUIDs copied");
}
