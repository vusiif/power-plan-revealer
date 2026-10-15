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
#include <QCloseEvent>
#include <QSignalBlocker>
#include <QSet>

#include <windows.h>
#include <shellapi.h>

#include <string>

namespace {

const QStringList kGamingPresetGuids = {
    "0CC5B647-C1DF-4637-891A-DEC35C318583",
    "EA062031-0E34-4FF1-9B6D-EB10593AC08F",
    "2DDD5A0F-0B42-4710-BC5D-08B504E6A659",
    "BE337238-0D82-4146-A960-4F3749D470C7",
    "45BCC021-7D09-4350-9E02-4F514D3CDD7D",
    "5D3E9A59-E9D5-4B00-A6BD-FF34FF516548",
    "5D3E9A59-E9D5-4B00-A6BD-FF34FF516547",
    "8BAA4A4D-1616-4562-B01F-22C5635B637D",
    "893DEE8E-2BEF-41E0-89C6-B55D0929964C",
    "BC5038F7-23E0-4960-96DA-3373F009F631",
    "94D3A615-A899-4AC5-8282-3E52B52AC837",
    "EE12F906-D277-404B-B6DA-E5FA1A576DF5",
    "A1662AB2-9D34-4E53-BA8B-2639B9E20857"
};

const QStringList kBatteryPresetGuids = {
    "12BBEBE6-58D6-4636-95BB-3217EF867C1A",
    "A9A07D01-E737-4DDA-9AE8-124B73B73134",
    "FBD9AA66-9553-4097-BA44-ED6E9D65EAB8",
    "13D09884-F74E-474A-A852-B6BDE8AD03A8",
    "5C5BB349-AD29-4EE2-9D0B-2B25270F7A81",
    "E69653CA-CF7F-4F05-AA73-CB833FA90AD4",
    "BC5038F7-23E0-4960-96DA-3373F009F631",
    "893DEE8E-2BEF-41E0-89C6-B55D0929964C",
    "BE337238-0D82-4146-A960-4F3749D470C7",
    "6738E2C4-E8A5-4142-F12D-64192F7A195E",
    "48E6B7A6-50F5-4782-A5D4-53BB8F07E226",
    "29F6C1DB-86DA-48C5-9FDB-F2B67B1F44DA",
    "9D781570-7042-4E7E-81B0-700E07444D93"
};

const QStringList kAudioPresetGuids = {
    "5D3E9A59-E9D5-4B00-A6BD-FF34FF516547",
    "5D3E9A59-E9D5-4B00-A6BD-FF34FF516548",
    "0CC5B647-C1DF-4637-891A-DEC35C318583",
    "EA062031-0E34-4FF1-9B6D-EB10593AC08F",
    "893DEE8E-2BEF-41E0-89C6-B55D0929964C",
    "BC5038F7-23E0-4960-96DA-3373F009F631",
    "94D3A615-A899-4AC5-8282-3E52B52AC837",
    "EE12F906-D277-404B-B6DA-E5FA1A576DF5",
    "48E6B7A6-50F5-4782-A5D4-53BB8F07E226"
};

const QSet<QString> kDefaultVisibleGuids = {
    "6738E2C4-E8A5-4142-F12D-64192F7A195E",
    "309D9467-7421-4107-87DA-4D64C964D087",
    "12BBEBE6-58D6-4636-95BB-3217EF867C1A",
    "29F6C1DB-86DA-48C5-9FDB-F2B67B1F44DA",
    "94ACFC8E-67EE-4505-81AC-341A67C7E1B8",
    "9D781570-7042-4E7E-81B0-700E07444D93",
    "BD3B718A-0680-4D9D-8C66-2708A3074B95",
    "5CA83367-6E45-459F-A27B-476B1D01C936",
    "7648E789-20C3-43F0-85DE-D072239FC611",
    "964446CBD15E4E2CB2F5D0F3239DA251",
    "EE12F906-D277-404B-B6DA-E5FA1A576DF5",
    "893DEE8E-2BEF-41E0-89C6-B55D0929964C",
    "94D3A615-A899-4AC5-8282-3E52B52AC837",
    "BC5038F7-23E0-4960-96DA-3373F009F631",
    "3C0BC021-C8A8-4E07-A973-6B14CBCB2B7E",
    "ADED5E82-B909-4619-9949-F5D71DAC0BCB",
    "F1FBFDE2-A960-4165-9F88-50667911CE96",
    "FBD9AA66-9553-4097-BA44-ED6E9D65EAB8",
    "03680956-93BC-4294-BBA6-4E0F09BB717F",
    "10778347-1370-4EE0-8BBD-33BDACAADE49",
    "34C7B99F-9A6D-4B3C-8DC7-B6693B78CEF4",
    "5DBB7C9F-38E9-40D2-9749-4F8A0E9F640F",
    "637EA02F-BBCB-4015-8E2C-A1C7B9C0B546",
    "8183BA9A-E910-48DA-8769-14AE6DC1170A",
    "9A66D8D7-4FF7-4EF9-B5A2-5A326CA2A469",
    "BCDED951-187B-4D05-BCCC-F7E51960C258",
    "D8742DCB-3E6A-4B3C-B3FE-374623CDCF06",
    "F3C5027D-CD16-4930-AA6B-90DB844A8F00"
};

} // namespace

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
      openControlPanelButton(nullptr),
      presetButton(nullptr),
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

    readSettings();

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
    setMinimumSize(880, 520);

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

    openControlPanelButton = new QPushButton("⚙ 打开高级电源设置", this);
    openControlPanelButton->setObjectName("PrimaryButton");
    openControlPanelButton->setCursor(Qt::PointingHandCursor);
    openControlPanelButton->setToolTip("直接呼出 Windows 原生“电源选项 - 高级设置”窗口，在此微调各选项的参数数值");

    schemeLayout->addWidget(schemeTitle);
    schemeLayout->addWidget(schemeComboBox);
    schemeLayout->addWidget(switchSchemeButton);
    schemeLayout->addWidget(activeSchemeLabel);
    schemeLayout->addStretch(1);
    schemeLayout->addWidget(openControlPanelButton);

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

    presetButton = new QPushButton("⚡ 场景预设与恢复", this);
    presetButton->setObjectName("SecondaryButton");
    presetButton->setCursor(Qt::PointingHandCursor);
    presetButton->setToolTip("一键批量解锁针对特定场景（电竞、笔电续航、音频防降频）的电源设置，或恢复系统默认显隐状态");

    auto *presetMenu = new QMenu(presetButton);
    presetMenu->addAction("🎮 电竞低延迟包 (CPU核心停车/睿频/PCIe节能)", this, [this]() {
        applyPreset("电竞低延迟包", kGamingPresetGuids);
    });
    presetMenu->addAction("🔋 笔电极限续航包 (Wi-Fi节能/自适应亮度/降频省电)", this, [this]() {
        applyPreset("笔电极限续航包", kBatteryPresetGuids);
    });
    presetMenu->addAction("🎵 音频生产力防降频包 (防DPC延迟/锁频防爆音/USB不挂起)", this, [this]() {
        applyPreset("音频生产力防降频包", kAudioPresetGuids);
    });
    presetMenu->addSeparator();
    presetMenu->addAction("🔄 恢复出厂默认显隐状态 (约20项标准可见，其余隐藏)", this, &MainWindow::resetToDefaultVisibility);
    presetMenu->addSeparator();
    presetMenu->addAction("🔓 全部取消隐藏 (解锁所有高级电源设置)", this, [this]() {
        setAllSettingsHidden(false);
    });
    presetMenu->addAction("🔒 全部隐藏 (隐藏所有高级电源设置)", this, [this]() {
        setAllSettingsHidden(true);
    });
    presetButton->setMenu(presetMenu);

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
        backupButton,
        presetButton
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
    topLayout->addWidget(presetButton);
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
    tree->header()->setSectionResizeMode(ColumnName, QHeaderView::Interactive);
    tree->header()->setSectionResizeMode(ColumnHidden, QHeaderView::Interactive);
    tree->header()->setSectionResizeMode(ColumnGuid, QHeaderView::Interactive);

    mainLayout->addWidget(heroFrame);
    mainLayout->addWidget(toolbarFrame);
    mainLayout->addWidget(tree, 1);

    setCentralWidget(central);

    connect(
        openControlPanelButton,
        &QPushButton::clicked,
        this,
        &MainWindow::openNativePowerSettings
    );

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

void MainWindow::closeEvent(QCloseEvent *event) {
    writeSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::readSettings() {
    QSettings settings("PowerPlanRevealer", "PowerPlanRevealer");

    const QByteArray geometry = settings.value("window/geometry").toByteArray();
    if (geometry.isEmpty()) {
        resize(1200, 700);
    } else {
        restoreGeometry(geometry);
    }

    const QByteArray headerState = settings.value("window/treeHeaderState").toByteArray();
    if (!headerState.isEmpty()) {
        tree->header()->restoreState(headerState);
    } else {
        tree->setColumnWidth(ColumnName, 620);
        tree->setColumnWidth(ColumnHidden, 90);
        tree->setColumnWidth(ColumnGuid, 420);
    }

    if (settings.contains("filter/onlyHidden")) {
        const QSignalBlocker blocker(onlyHiddenCheckBox);
        onlyHiddenCheckBox->setChecked(settings.value("filter/onlyHidden").toBool());
    }
}

void MainWindow::writeSettings() {
    QSettings settings("PowerPlanRevealer", "PowerPlanRevealer");
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/treeHeaderState", tree->header()->saveState());
    settings.setValue("filter/onlyHidden", onlyHiddenCheckBox->isChecked());
}

void MainWindow::openNativePowerSettings() {
    HINSTANCE res = ShellExecuteW(
        NULL,
        L"open",
        L"control.exe",
        L"powercfg.cpl,,1",
        NULL,
        SW_SHOWNORMAL
    );

    if (reinterpret_cast<INT_PTR>(res) <= 32) {
        QMessageBox::warning(
            this,
            "打开失败",
            "未能成功启动 Windows 控制面板。您可以按 Win+R 手动运行：\ncontrol.exe powercfg.cpl,,1"
        );
    } else {
        statusLabel->setText("已打开高级设置");
    }
}

void MainWindow::applyPreset(const QString &presetName, const QStringList &targetGuids) {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "应用场景预设",
        QString("将为【%1】批量解锁对应的 %2 项高级电源设置。\n\n"
                "• 作用：将相关选项在 Windows 控制面板中设置为“可见”。\n"
                "• 安全提示：绝不会修改您的任何电源参数数值。\n\n"
                "确定要继续吗？")
            .arg(presetName)
            .arg(targetGuids.size()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    QSet<QString> targetSet;
    for (const QString &guid : targetGuids) {
        targetSet.insert(guid.trimmed().toUpper());
    }

    int updatedCount = 0;

    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *subgroupItem = tree->topLevelItem(i);
        const QString subgroupGuidStr = subgroupItem->data(ColumnName, RoleSubgroupGuid).toString();
        GUID subgroupGuid;
        const std::wstring subgroupWide = subgroupGuidStr.toStdWString();
        if (!parse_guid_text(subgroupWide.c_str(), &subgroupGuid)) {
            continue;
        }

        for (int j = 0; j < subgroupItem->childCount(); ++j) {
            QTreeWidgetItem *settingItem = subgroupItem->child(j);
            const QString settingGuidStr = settingItem->data(ColumnName, RoleSettingGuid).toString().trimmed().toUpper();

            if (targetSet.contains(settingGuidStr)) {
                GUID settingGuid;
                const std::wstring settingWide = settingGuidStr.toStdWString();
                if (parse_guid_text(settingWide.c_str(), &settingGuid)) {
                    if (unhide_setting(&subgroupGuid, &settingGuid) == ERROR_SUCCESS) {
                        updatedCount++;
                    }
                }
            }
        }
    }

    refresh_active_scheme();
    reloadTree();

    statusLabel->setText(QString("预设已应用 (%1 项)").arg(updatedCount));

    QMessageBox::information(
        this,
        "预设应用完成",
        QString("【%1】已应用！共成功解锁并公开了 %2 项电源设置。\n\n"
                "您可以点击界面上的“⚙ 打开高级电源设置”按钮立即在控制面板中调参。")
            .arg(presetName)
            .arg(updatedCount)
    );
}

void MainWindow::resetToDefaultVisibility() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "恢复出厂默认显隐状态",
        "确定要将所有电源设置项的显隐状态恢复为 Windows 官方出厂默认吗？\n\n"
        "• 约 20 项常用标准设置保持可见（如睡眠时间、关闭显示、处理器状态等）\n"
        "• 其余所有高级设置将恢复为“隐藏”状态\n"
        "• 注意：本操作仅恢复显示/隐藏属性，绝不会改动您的任何具体电源数值。\n\n"
        "是否继续？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    int hideCount = 0;
    int unhideCount = 0;

    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *subgroupItem = tree->topLevelItem(i);
        const QString subgroupGuidStr = subgroupItem->data(ColumnName, RoleSubgroupGuid).toString();
        GUID subgroupGuid;
        const std::wstring subgroupWide = subgroupGuidStr.toStdWString();
        if (!parse_guid_text(subgroupWide.c_str(), &subgroupGuid)) {
            continue;
        }

        for (int j = 0; j < subgroupItem->childCount(); ++j) {
            QTreeWidgetItem *settingItem = subgroupItem->child(j);
            const QString settingGuidStr = settingItem->data(ColumnName, RoleSettingGuid).toString().trimmed().toUpper();
            GUID settingGuid;
            const std::wstring settingWide = settingGuidStr.toStdWString();
            if (!parse_guid_text(settingWide.c_str(), &settingGuid)) {
                continue;
            }

            if (kDefaultVisibleGuids.contains(settingGuidStr)) {
                if (unhide_setting(&subgroupGuid, &settingGuid) == ERROR_SUCCESS) {
                    unhideCount++;
                }
            } else {
                if (hide_setting(&subgroupGuid, &settingGuid) == ERROR_SUCCESS) {
                    hideCount++;
                }
            }
        }
    }

    refresh_active_scheme();
    reloadTree();

    statusLabel->setText("已恢复默认显隐");

    QMessageBox::information(
        this,
        "已恢复默认显隐状态",
        QString("已成功恢复 Windows 原厂出厂显隐状态！\n\n"
                "• 保持可见项：%1 项\n"
                "• 恢复隐藏项：%2 项\n\n"
                "所有参数数值均保持原样未变。")
            .arg(unhideCount)
            .arg(hideCount)
    );
}

void MainWindow::setAllSettingsHidden(bool hidden) {
    const QString actionName = hidden ? "全部隐藏" : "全部取消隐藏";

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        actionName,
        QString("确定要将系统中所有的电源设置项【%1】吗？\n\n"
                "• 提示：这只会修改设置项在控制面板中的显隐属性，不会修改任何参数数值。\n\n"
                "是否继续？")
            .arg(actionName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    int count = 0;

    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *subgroupItem = tree->topLevelItem(i);
        const QString subgroupGuidStr = subgroupItem->data(ColumnName, RoleSubgroupGuid).toString();
        GUID subgroupGuid;
        const std::wstring subgroupWide = subgroupGuidStr.toStdWString();
        if (!parse_guid_text(subgroupWide.c_str(), &subgroupGuid)) {
            continue;
        }

        for (int j = 0; j < subgroupItem->childCount(); ++j) {
            QTreeWidgetItem *settingItem = subgroupItem->child(j);
            const QString settingGuidStr = settingItem->data(ColumnName, RoleSettingGuid).toString();
            GUID settingGuid;
            const std::wstring settingWide = settingGuidStr.toStdWString();
            if (!parse_guid_text(settingWide.c_str(), &settingGuid)) {
                continue;
            }

            const DWORD rc = hidden
                ? hide_setting(&subgroupGuid, &settingGuid)
                : unhide_setting(&subgroupGuid, &settingGuid);

            if (rc == ERROR_SUCCESS) {
                count++;
            }
        }
    }

    refresh_active_scheme();
    reloadTree();

    statusLabel->setText(hidden ? "已全部隐藏" : "已全部解锁");

    QMessageBox::information(
        this,
        "完成",
        QString("操作完成！已成功对 %1 项设置执行【%2】。").arg(count).arg(actionName)
    );
}

