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
#include <QFile>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QDateTime>
#include <QComboBox>
#include <QStyleHints>
#include <QGuiApplication>
#include <QPalette>
#include <QColor>
#include <QStyle>

#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      tree(nullptr),
      searchEdit(nullptr),
      refreshButton(nullptr),
      unhideButton(nullptr),
      hideButton(nullptr),
      copyGuidButton(nullptr),
      backupButton(nullptr),
      onlyHiddenCheckBox(nullptr),
      schemeComboBox(nullptr),
      switchSchemeButton(nullptr),
      activeSchemeLabel(nullptr),
      statusLabel(nullptr),
      totalSettingCount(0) {
    setupUi();

    const bool isDark = (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
    loadStyleSheet(isDark);

    connect(
        QGuiApplication::styleHints(),
        &QStyleHints::colorSchemeChanged,
        this,
        [this](Qt::ColorScheme scheme) {
            loadStyleSheet(scheme == Qt::ColorScheme::Dark);
        }
    );

    showStartupSafetyNotice();
    loadSchemes();
    reloadTree();
}

void MainWindow::loadStyleSheet(bool isDark) {
    if (isDark) {
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(15, 23, 42));
        darkPalette.setColor(QPalette::WindowText, QColor(248, 250, 252));
        darkPalette.setColor(QPalette::Base, QColor(30, 41, 59));
        darkPalette.setColor(QPalette::AlternateBase, QColor(22, 32, 50));
        darkPalette.setColor(QPalette::ToolTipBase, QColor(15, 23, 42));
        darkPalette.setColor(QPalette::ToolTipText, QColor(248, 250, 252));
        darkPalette.setColor(QPalette::Text, QColor(248, 250, 252));
        darkPalette.setColor(QPalette::Button, QColor(51, 65, 85));
        darkPalette.setColor(QPalette::ButtonText, QColor(248, 250, 252));
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(96, 165, 250));
        darkPalette.setColor(QPalette::Highlight, QColor(37, 99, 235));
        darkPalette.setColor(QPalette::HighlightedText, Qt::white);
        qApp->setPalette(darkPalette);
    } else {
        qApp->setPalette(qApp->style()->standardPalette());
    }

    const QString resourcePath = isDark ? ":/style-dark.qss" : ":/style-light.qss";
    QFile file(resourcePath);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
    }
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
    heroLayout->setSpacing(8);

    auto *heroTitle = new QLabel("PowerPlanRevealer", this);
    heroTitle->setObjectName("HeroTitle");

    auto *heroSubtitle = new QLabel(
        "查看、搜索并管理 Windows 电源计划高级设置的隐藏状态",
        this
    );
    heroSubtitle->setObjectName("HeroSubtitle");
    heroSubtitle->setWordWrap(true);

    auto *schemeLayout = new QHBoxLayout();
    schemeLayout->setSpacing(8);

    auto *schemeTitle = new QLabel("电源计划方案：", this);
    schemeTitle->setObjectName("SchemeTitle");

    schemeComboBox = new QComboBox(this);
    schemeComboBox->setObjectName("SchemeComboBox");
    schemeComboBox->setCursor(Qt::PointingHandCursor);

    switchSchemeButton = new QPushButton("设为当前活动方案", this);
    switchSchemeButton->setObjectName("PrimaryButton");
    switchSchemeButton->setCursor(Qt::PointingHandCursor);
    switchSchemeButton->setToolTip("将下拉框中选中的电源计划激活为系统当前运行的方案");

    activeSchemeLabel = new QLabel(this);
    activeSchemeLabel->setObjectName("ActiveSchemeLabel");

    schemeLayout->addWidget(schemeTitle);
    schemeLayout->addWidget(schemeComboBox);
    schemeLayout->addWidget(switchSchemeButton);
    schemeLayout->addWidget(activeSchemeLabel);
    schemeLayout->addStretch(1);

    heroLayout->addWidget(heroTitle);
    heroLayout->addWidget(heroSubtitle);
    heroLayout->addLayout(schemeLayout);

    auto *toolbarFrame = new QFrame(this);
    toolbarFrame->setObjectName("ToolbarFrame");

    auto *topLayout = new QHBoxLayout(toolbarFrame);
    topLayout->setContentsMargins(12, 10, 12, 10);
    topLayout->setSpacing(8);

    searchEdit = new QLineEdit(this);
    searchEdit->setObjectName("SearchEdit");
    searchEdit->setPlaceholderText("搜索名称、GUID 或说明关键字...");
    searchEdit->setClearButtonEnabled(true);

    refreshButton = new QPushButton("刷新", this);
    unhideButton = new QPushButton("取消隐藏所选项", this);
    hideButton = new QPushButton("隐藏所选项", this);
    copyGuidButton = new QPushButton("复制 GUID", this);
    backupButton = new QPushButton("备份注册表", this);

    refreshButton->setObjectName("SecondaryButton");
    unhideButton->setObjectName("PrimaryButton");
    hideButton->setObjectName("DangerButton");
    copyGuidButton->setObjectName("SecondaryButton");
    backupButton->setObjectName("SecondaryButton");

    backupButton->setToolTip("导出当前系统电源设置注册表项为 .reg 备份文件");

    QPushButton *actionButtons[] = {
        refreshButton,
        unhideButton,
        hideButton,
        copyGuidButton,
        backupButton
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
    topLayout->addWidget(backupButton);
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
    tree->setContextMenuPolicy(Qt::CustomContextMenu);

    tree->header()->setStretchLastSection(false);
    tree->header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    tree->header()->setSectionResizeMode(ColumnName, QHeaderView::Stretch);
    tree->header()->setSectionResizeMode(ColumnHidden, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(ColumnGuid, QHeaderView::ResizeToContents);

    mainLayout->addWidget(heroFrame);
    mainLayout->addWidget(toolbarFrame);
    mainLayout->addWidget(tree, 1);

    setCentralWidget(central);

    connect(
        switchSchemeButton,
        &QPushButton::clicked,
        this,
        &MainWindow::switchActiveScheme
    );

    connect(
        refreshButton,
        &QPushButton::clicked,
        this,
        [this]() {
            loadSchemes();
            reloadTree();
        }
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
        backupButton,
        &QPushButton::clicked,
        this,
        &MainWindow::backupRegistry
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

    connect(
        tree,
        &QTreeWidget::customContextMenuRequested,
        this,
        &MainWindow::showTreeContextMenu
    );

    connect(
        tree,
        &QTreeWidget::itemDoubleClicked,
        this,
        &MainWindow::onTreeItemDoubleClicked
    );
}

int MainWindow::schemeEnumerateCallback(
    const PowerSchemeItem *item,
    void *userData
) {
    auto *window = static_cast<MainWindow *>(userData);
    if (window == nullptr || item == nullptr) {
        return 0;
    }

    const QString name = QString::fromWCharArray(item->scheme_name);
    const QString guid = QString::fromWCharArray(item->scheme_guid_text);
    const QString text = item->is_active ? QString("%1 (当前激活)").arg(name) : name;

    window->schemeComboBox->addItem(text, guid);
    if (item->is_active) {
        window->schemeComboBox->setCurrentIndex(window->schemeComboBox->count() - 1);
    }

    return 0;
}

void MainWindow::loadSchemes() {
    schemeComboBox->blockSignals(true);
    schemeComboBox->clear();
    enumerate_power_schemes(MainWindow::schemeEnumerateCallback, this);
    schemeComboBox->blockSignals(false);
    updateActiveSchemeDisplay();
}

void MainWindow::switchActiveScheme() {
    const QString guidText = schemeComboBox->currentData().toString();
    if (guidText.isEmpty()) {
        return;
    }

    GUID schemeGuid;
    const std::wstring wideGuid = guidText.toStdWString();
    if (!parse_guid_text(wideGuid.c_str(), &schemeGuid)) {
        QMessageBox::critical(this, "错误", "无效的电源方案 GUID。");
        return;
    }

    const DWORD rc = set_active_scheme_by_guid(&schemeGuid);
    if (rc != ERROR_SUCCESS) {
        QString msg = QString("切换方案失败，错误代码：%1").arg(rc);
        if (rc == ERROR_ACCESS_DENIED) {
            msg += "\n\n访问被拒绝。请确认已以管理员身份运行。";
        }
        QMessageBox::critical(this, "操作失败", msg);
        return;
    }

    loadSchemes();
    reloadTree();
    statusLabel->setText("方案已激活");
}

void MainWindow::updateActiveSchemeDisplay() {
    wchar_t schemeName[PPR_NAME_CCH];
    DWORD rc = get_active_scheme_name(schemeName, PPR_NAME_CCH);
    if (rc == ERROR_SUCCESS) {
        activeSchemeLabel->setText(
            QString("当前激活：%1").arg(QString::fromWCharArray(schemeName))
        );
    } else {
        activeSchemeLabel->setText("当前激活：未知");
    }
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
        "或点击工具栏上的“备份注册表”导出备份。\n\n"
        "注册表路径：\n"
        "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerSettings\n\n"
        "本工具只会修改所选电源设置项的隐藏属性（hide attribute），"
        "不会擅自修改电源设置的具体数值。"
        "不过这些设置属于系统级，修改前请确保了解所操作的项目。"
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

    updateActiveSchemeDisplay();
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
    const QString description = QString::fromWCharArray(item->description);
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
    settingItem->setData(ColumnName, RoleDescription, description);
    settingItem->setData(ColumnName, RoleHasAc, item->has_ac_value);
    settingItem->setData(ColumnName, RoleAcValue, item->has_ac_value ? static_cast<quint32>(item->ac_value) : 0u);
    settingItem->setData(ColumnName, RoleHasDc, item->has_dc_value);
    settingItem->setData(ColumnName, RoleDcValue, item->has_dc_value ? static_cast<quint32>(item->dc_value) : 0u);

    QString tooltip = QString("<b>%1</b><br/><b>状态：</b>%2<br/><b>GUID：</b>%3")
        .arg(settingName, item->hidden ? "隐藏" : "可见", settingGuid);

    if (item->has_ac_value || item->has_dc_value) {
        tooltip += "<br/><br/><b>当前配置值：</b>";
        if (item->has_ac_value) {
            tooltip += QString("<br/>• 接通电源 (AC)：%1 (0x%2)")
                .arg(item->ac_value)
                .arg(QString::number(item->ac_value, 16).toUpper().rightJustified(8, '0'));
        }
        if (item->has_dc_value) {
            tooltip += QString("<br/>• 使用电池 (DC)：%1 (0x%2)")
                .arg(item->dc_value)
                .arg(QString::number(item->dc_value, 16).toUpper().rightJustified(8, '0'));
        }
    }

    if (!description.isEmpty()) {
        tooltip += QString("<br/><br/><b>说明：</b><br/>%1").arg(description);
    }

    settingItem->setToolTip(ColumnName, tooltip);
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

            const QString desc = settingItem->data(ColumnName, RoleDescription).toString();
            const bool settingMatches =
                settingItem->text(ColumnName).contains(query, Qt::CaseInsensitive) ||
                settingItem->text(ColumnHidden).contains(query, Qt::CaseInsensitive) ||
                settingItem->text(ColumnGuid).contains(query, Qt::CaseInsensitive) ||
                desc.contains(query, Qt::CaseInsensitive);

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
            "建议在修改前先创建系统还原点，或使用“备份注册表”导出备份。"
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
            message += "\n\n访问被拒绝。请确认程序已以管理员身份运行。";
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

void MainWindow::copySelectedName() {
    QTreeWidgetItem *item = selectedSettingItem();
    if (item != nullptr) {
        QApplication::clipboard()->setText(item->text(ColumnName));
        statusLabel->setText("名称已复制");
    }
}

void MainWindow::copySelectedRegistryPath() {
    QTreeWidgetItem *item = selectedSettingItem();
    if (item != nullptr) {
        const QString subgroupGuid = item->data(ColumnName, RoleSubgroupGuid).toString();
        const QString settingGuid = item->data(ColumnName, RoleSettingGuid).toString();
        const QString regPath = QString(
            "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerSettings\\%1\\%2"
        ).arg(subgroupGuid, settingGuid);
        QApplication::clipboard()->setText(regPath);
        statusLabel->setText("注册表路径已复制");
    }
}

void MainWindow::openSelectedInRegedit() {
    QTreeWidgetItem *item = selectedSettingItem();
    if (item == nullptr) {
        return;
    }
    const QString subgroupGuid = item->data(ColumnName, RoleSubgroupGuid).toString();
    const QString settingGuid = item->data(ColumnName, RoleSettingGuid).toString();
    open_in_regedit(subgroupGuid.toStdWString().c_str(), settingGuid.toStdWString().c_str());
    statusLabel->setText("已打开注册表编辑器");
}

void MainWindow::showTreeContextMenu(const QPoint &pos) {
    QTreeWidgetItem *item = tree->itemAt(pos);
    if (item == nullptr) {
        return;
    }

    const int itemType = item->data(ColumnName, RoleItemType).toInt();
    QMenu menu(this);

    if (itemType == ItemTypeSetting) {
        const bool hidden = item->data(ColumnName, RoleHidden).toInt() != 0;
        menu.addAction(
            hidden ? "取消隐藏此项" : "隐藏此项",
            this,
            [this, hidden]() {
                setSelectedSettingHidden(!hidden);
            }
        );

        menu.addSeparator();

        menu.addAction("复制设置项名称", this, &MainWindow::copySelectedName);
        menu.addAction("复制 GUID (Subgroup & Setting)", this, &MainWindow::copySelectedGuids);
        menu.addAction("复制完整注册表路径", this, &MainWindow::copySelectedRegistryPath);
        menu.addAction("在注册表编辑器中定位", this, &MainWindow::openSelectedInRegedit);
    } else if (itemType == ItemTypeSubgroup) {
        const QString subgroupName = item->text(ColumnName);
        const QString subgroupGuid = item->data(ColumnName, RoleSubgroupGuid).toString();
        menu.addAction("复制分类名称", this, [subgroupName]() {
            QApplication::clipboard()->setText(subgroupName);
        });
        menu.addAction("复制 Subgroup GUID", this, [subgroupGuid]() {
            QApplication::clipboard()->setText(subgroupGuid);
        });
        menu.addAction("在注册表编辑器中定位分类", this, [subgroupGuid]() {
            open_in_regedit(subgroupGuid.toStdWString().c_str(), NULL);
        });
    }

    menu.exec(tree->viewport()->mapToGlobal(pos));
}

void MainWindow::onTreeItemDoubleClicked(QTreeWidgetItem *item, int column) {
    Q_UNUSED(column);
    if (item == nullptr) {
        return;
    }
    const int itemType = item->data(ColumnName, RoleItemType).toInt();
    if (itemType == ItemTypeSetting) {
        const bool hidden = item->data(ColumnName, RoleHidden).toInt() != 0;
        setSelectedSettingHidden(!hidden);
    }
}

void MainWindow::backupRegistry() {
    const QString defaultFileName = QString("PowerSettings_backup_%1.reg")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "导出电源设置注册表备份",
        defaultFileName,
        "注册表脚本 (*.reg)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    const int rc = backup_power_settings_registry(filePath.toStdWString().c_str());
    if (rc == 0) {
        QMessageBox::information(
            this,
            "备份成功",
            QString("电源设置注册表已成功导出备份至：\n%1").arg(filePath)
        );
        statusLabel->setText("注册表已备份");
    } else {
        QString message = QString("导出注册表备份失败。错误代码：%1").arg(rc);
        if (rc == ERROR_ACCESS_DENIED) {
            message += "\n\n访问被拒绝。请确认程序已具备管理员权限。";
        }
        QMessageBox::critical(
            this,
            "备份失败",
            message
        );
    }
}
