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
#include <QFrame>

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
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(12);

    auto *heroFrame = new QFrame(this);
    heroFrame->setObjectName("HeroFrame");

    auto *heroLayout = new QVBoxLayout(heroFrame);
    heroLayout->setContentsMargins(18, 14, 18, 14);
    heroLayout->setSpacing(4);

    auto *heroTitle = new QLabel("PowerPlanRevealer", this);
    heroTitle->setObjectName("HeroTitle");

    auto *heroSubtitle = new QLabel(
        "查看、搜索并管理 Windows 电源计划高级设置的隐藏状态",
        this
    );
    heroSubtitle->setObjectName("HeroSubtitle");
    heroSubtitle->setWordWrap(true);

    heroLayout->addWidget(heroTitle);
    heroLayout->addWidget(heroSubtitle);

    auto *toolbarFrame = new QFrame(this);
    toolbarFrame->setObjectName("ToolbarFrame");

    auto *topLayout = new QHBoxLayout(toolbarFrame);
    topLayout->setContentsMargins(12, 10, 12, 10);
    topLayout->setSpacing(8);

    searchEdit = new QLineEdit(this);
    searchEdit->setObjectName("SearchEdit");
    searchEdit->setPlaceholderText("搜索名称或 GUID...");
    searchEdit->setClearButtonEnabled(true);

    refreshButton = new QPushButton("刷新", this);
    unhideButton = new QPushButton("取消隐藏所选项", this);
    hideButton = new QPushButton("隐藏所选项", this);
    copyGuidButton = new QPushButton("复制 GUID", this);

    refreshButton->setObjectName("SecondaryButton");
    unhideButton->setObjectName("PrimaryButton");
    hideButton->setObjectName("DangerButton");
    copyGuidButton->setObjectName("SecondaryButton");

    QPushButton *actionButtons[] = {
        refreshButton,
        unhideButton,
        hideButton,
        copyGuidButton
    };

    for (QPushButton *button : actionButtons) {
        button->setCursor(Qt::PointingHandCursor);
    }

    unhideButton->setEnabled(false);
    hideButton->setEnabled(false);
    copyGuidButton->setEnabled(false);

    onlyHiddenCheckBox = new QCheckBox("仅显示隐藏项", this);
    onlyHiddenCheckBox->setObjectName("OnlyHiddenCheckBox");
    onlyHiddenCheckBox->setChecked(true);
    onlyHiddenCheckBox->setCursor(Qt::PointingHandCursor);

    statusLabel = new QLabel(this);
    statusLabel->setObjectName("StatusBadge");
    statusLabel->setMinimumWidth(118);
    statusLabel->setAlignment(Qt::AlignCenter);

    topLayout->addWidget(searchEdit, 1);
    topLayout->addWidget(refreshButton);
    topLayout->addWidget(unhideButton);
    topLayout->addWidget(hideButton);
    topLayout->addWidget(copyGuidButton);
    topLayout->addWidget(onlyHiddenCheckBox);
    topLayout->addWidget(statusLabel);

    tree = new QTreeWidget(this);
    tree->setObjectName("PowerTree");
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
    tree->header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    tree->header()->setSectionResizeMode(ColumnName, QHeaderView::Stretch);
    tree->header()->setSectionResizeMode(ColumnHidden, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(ColumnGuid, QHeaderView::ResizeToContents);

    mainLayout->addWidget(heroFrame);
    mainLayout->addWidget(toolbarFrame);
    mainLayout->addWidget(tree, 1);

    setCentralWidget(central);

    setStyleSheet(R"(
        QMainWindow {
            background: #eef2f7;
        }

        QWidget {
            font-family: "Microsoft YaHei UI", "Segoe UI";
            font-size: 13px;
            color: #111827;
        }

        QFrame#HeroFrame {
            background: qlineargradient(
                x1: 0, y1: 0,
                x2: 1, y2: 1,
                stop: 0 #ffffff,
                stop: 1 #e8f0ff
            );
            border: 1px solid #d7dee8;
            border-radius: 14px;
        }

        QLabel#HeroTitle {
            font-size: 22px;
            font-weight: 700;
            color: #111827;
        }

        QLabel#HeroSubtitle {
            color: #4b5563;
        }

        QFrame#ToolbarFrame {
            background: #ffffff;
            border: 1px solid #d8dee9;
            border-radius: 12px;
        }

        QLineEdit#SearchEdit {
            padding: 7px 12px;
            border: 1px solid #cfd8e3;
            border-radius: 8px;
            background: #f9fafb;
            selection-background-color: #bfdbfe;
        }

        QLineEdit#SearchEdit:focus {
            border: 1px solid #3b82f6;
            background: #ffffff;
        }

        QPushButton {
            padding: 7px 13px;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            background: #ffffff;
            color: #1f2937;
            font-weight: 500;
        }

        QPushButton:hover {
            background: #f8fafc;
            border-color: #94a3b8;
        }

        QPushButton#PrimaryButton {
            color: #ffffff;
            background: #2563eb;
            border-color: #1d4ed8;
        }

        QPushButton#PrimaryButton:hover {
            background: #1d4ed8;
            border-color: #1e40af;
        }

        QPushButton#DangerButton {
            color: #ffffff;
            background: #dc2626;
            border-color: #b91c1c;
        }

        QPushButton#DangerButton:hover {
            background: #b91c1c;
            border-color: #991b1b;
        }

        QPushButton:disabled,
        QPushButton#PrimaryButton:disabled,
        QPushButton#DangerButton:disabled {
            color: #9ca3af;
            background: #f3f4f6;
            border-color: #d1d5db;
        }

        QCheckBox#OnlyHiddenCheckBox {
            spacing: 7px;
            color: #374151;
            font-weight: 500;
        }

        QCheckBox#OnlyHiddenCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid #cbd5e1;
            border-radius: 4px;
            background: #ffffff;
        }

        QCheckBox#OnlyHiddenCheckBox::indicator:checked {
            background: #2563eb;
            border-color: #1d4ed8;
        }

        QLabel#StatusBadge {
            padding: 5px 10px;
            border: 1px solid #d8dee9;
            border-radius: 999px;
            background: #f8fafc;
            color: #475569;
            font-weight: 500;
        }

        QTreeWidget#PowerTree {
            background: #ffffff;
            border: 1px solid #d8dee9;
            border-radius: 12px;
            alternate-background-color: #f8fafc;
            outline: 0;
        }

        QTreeWidget#PowerTree::item {
            min-height: 28px;
            padding: 4px 6px;
            border-bottom: 1px solid #f1f5f9;
        }

        QTreeWidget#PowerTree::item:hover {
            background: #eff6ff;
        }

        QTreeWidget#PowerTree::item:selected {
            background: #dbeafe;
            color: #111827;
        }

        QHeaderView::section {
            padding: 8px 10px;
            background: #f1f5f9;
            color: #334155;
            border: none;
            border-right: 1px solid #e2e8f0;
            border-bottom: 1px solid #d8dee9;
            font-weight: 700;
        }

        QScrollBar:vertical {
            background: #f8fafc;
            width: 12px;
            margin: 2px;
        }

        QScrollBar::handle:vertical {
            background: #cbd5e1;
            border-radius: 6px;
            min-height: 28px;
        }

        QScrollBar::handle:vertical:hover {
            background: #94a3b8;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0;
        }

        QScrollBar:horizontal {
            background: #f8fafc;
            height: 12px;
            margin: 2px;
        }

        QScrollBar::handle:horizontal {
            background: #cbd5e1;
            border-radius: 6px;
            min-width: 28px;
        }

        QScrollBar::handle:horizontal:hover {
            background: #94a3b8;
        }

        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal {
            width: 0;
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
    ).arg(
        actionText,
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

    const QString beforeAttributesText = QString::number(
    beforeAttributes,
    16
).toUpper().rightJustified(8, '0');

    const QString afterAttributesText = QString::number(
        afterAttributes,
        16
    ).toUpper().rightJustified(8, '0');

    const QString hiddenFilterNotice =
        onlyHiddenCheckBox->isChecked() && !hidden
            ? "注意：当前启用了“仅显示隐藏项”，这个设置项取消隐藏后会从列表中消失。"
            : "";

    QMessageBox::information(
        this,
        "完成",
        QString(
            "%1\n\n"
            "设置项：\n%2\n\n"
            "修改前 attributes：0x%3\n"
            "修改后 attributes：0x%4\n\n"
            "%5"
        ).arg(
            hidden ? "设置项已经隐藏。" : "设置项已经取消隐藏。",
            settingName,
            beforeAttributesText,
            afterAttributesText,
            hiddenFilterNotice
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
