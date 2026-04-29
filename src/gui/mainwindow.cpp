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
#include <QCheckBox>
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
    searchEdit->setPlaceholderText("搜索名称或 GUID...");
    searchEdit->setClearButtonEnabled(true);

    refreshButton = new QPushButton("刷新", this);
    unhideButton = new QPushButton("取消隐藏所选项", this);
    hideButton = new QPushButton("隐藏所选项", this);
    copyGuidButton = new QPushButton("复制 GUID", this);

    unhideButton->setEnabled(false);
    hideButton->setEnabled(false);
    copyGuidButton->setEnabled(false);

    onlyHiddenCheckBox = new QCheckBox("仅显示隐藏项", this);
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
        "名称",
        "隐藏",
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
    box.setWindowTitle("安全提示");

    box.setText(
        "PowerPlanRevealer 可以修改 Windows 电源高级设置项的隐藏状态。"
    );

    box.setInformativeText(
        "在使用“隐藏”或“取消隐藏”功能之前，建议先创建系统还原点，"
        "或者导出相关注册表项作为备份。\n\n"
        "建议备份的注册表路径：\n"
        "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerSettings\n\n"
        "本工具只会修改所选电源设置项的 hide attribute，"
        "不会主动修改电源设置的具体数值。"
        "不过这些仍然属于系统级设置，修改前请确保你知道自己正在操作的项目。"
    );

    box.setStandardButtons(QMessageBox::Ok);
    box.setDefaultButton(QMessageBox::Ok);

    auto *dontShowAgain = new QCheckBox(
        "不再显示此提示",
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
            "错误",
            QString("枚举电源设置失败。错误代码：%1").arg(rc)
        );

        statusLabel->setText("失败");
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
    const QString hiddenText = item->hidden ? "是" : "否";

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
            QString("显示 %1 / %2 项，%3 个分类")
                .arg(visibleSettingCount)
                .arg(totalSettingCount)
                .arg(visibleSubgroupCount)
        );
    } else {
        statusLabel->setText(
            QString("%1 项，%2 个分类")
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
            "未选择设置项",
            "请先选择一个具体的电源设置项。"
        );
        return;
    }

    const bool currentlyHidden = item->data(ColumnName, RoleHidden).toInt() != 0;

    if (currentlyHidden == hidden) {
        QMessageBox::information(
            this,
            "无需修改",
            hidden
                ? "这个设置项当前已经是隐藏状态。"
                : "这个设置项当前已经是可见状态。"
        );
        return;
    }

    const QString settingName = item->text(ColumnName);
    const QString subgroupGuidText = item->data(ColumnName, RoleSubgroupGuid).toString();
    const QString settingGuidText = item->data(ColumnName, RoleSettingGuid).toString();

    const QString actionText = hidden ? "隐藏" : "取消隐藏";
    const QString currentStateText = currentlyHidden ? "隐藏" : "可见";
    const QString targetStateText = hidden ? "隐藏" : "可见";

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        hidden ? "确认隐藏设置项" : "确认取消隐藏设置项",
        QString(
            "请确认以下修改：\n\n"
            "操作：%1\n\n"
            "设置项：\n%2\n\n"
            "当前状态：%3\n"
            "目标状态：%4\n\n"
            "Subgroup GUID：\n%5\n\n"
            "Setting GUID：\n%6\n\n"
            "建议在修改前先创建系统还原点，或者备份以下注册表路径：\n"
            "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerSettings"
        )
            .arg(actionText)
            .arg(settingName)
            .arg(currentStateText)
            .arg(targetStateText)
            .arg(subgroupGuidText)
            .arg(settingGuidText),
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
            "无效 GUID",
            QString("Subgroup GUID 无效：\n%1").arg(subgroupGuidText)
        );
        return;
    }

    if (!parse_guid_text(settingWide.c_str(), &settingGuid)) {
        QMessageBox::critical(
            this,
            "无效 GUID",
            QString("Setting GUID 无效：\n%1").arg(settingGuidText)
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
        QString message = QString("操作失败。错误代码：%1").arg(rc);

        if (rc == ERROR_ACCESS_DENIED) {
            message += "\n\n访问被拒绝。请确认 GUI 已经以管理员身份运行。";
        }

        QMessageBox::critical(
            this,
            "操作失败",
            message
        );

        return;
    }

    const DWORD refreshRc = refresh_active_scheme();

    if (refreshRc != ERROR_SUCCESS) {
        QMessageBox::warning(
            this,
            "刷新当前电源计划失败",
            QString("设置项已经修改成功，但刷新当前电源计划失败。错误代码：%1")
                .arg(refreshRc)
        );
    }

    QMessageBox::information(
        this,
        "完成",
        QString(
            "%1\n\n"
            "设置项：\n%2\n\n"
            "修改前 attributes：0x%3\n"
            "修改后 attributes：0x%4\n\n"
            "%5"
        )
            .arg(hidden ? "设置项已经隐藏。" : "设置项已经取消隐藏。")
            .arg(settingName)
            .arg(QString::number(beforeAttributes, 16).toUpper().rightJustified(8, '0'))
            .arg(QString::number(afterAttributes, 16).toUpper().rightJustified(8, '0'))
            .arg(
                onlyHiddenCheckBox->isChecked() && !hidden
                    ? "注意：当前启用了“仅显示隐藏项”，这个设置项取消隐藏后会从列表中消失。"
                    : ""
            )
    );

    reloadTree();

    statusLabel->setText(
        hidden ? "设置项已隐藏" : "设置项已取消隐藏"
    );
}

void MainWindow::copySelectedGuids() {
    QTreeWidgetItem *item = selectedSettingItem();

    if (item == nullptr) {
        QMessageBox::information(
            this,
            "未选择设置项",
            "请先选择一个具体的电源设置项。"
        );
        return;
    }

    const QString subgroupGuidText = item->data(ColumnName, RoleSubgroupGuid).toString();
    const QString settingGuidText = item->data(ColumnName, RoleSettingGuid).toString();

    const QString text = QString("Subgroup GUID: %1\nSetting GUID: %2")
        .arg(subgroupGuidText, settingGuidText);

    QApplication::clipboard()->setText(text);

    statusLabel->setText("GUID 已复制");
}
