#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QSettings>
#include <QCheckBox>

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
