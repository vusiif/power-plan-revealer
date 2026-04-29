#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      table(nullptr),
      refreshButton(nullptr),
      onlyHiddenCheckBox(nullptr),
      statusLabel(nullptr) {
    setupUi();
    reloadTable();
}

void MainWindow::setupUi() {
    setWindowTitle("PowerPlanRevealer");

    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    auto *topLayout = new QHBoxLayout();

    refreshButton = new QPushButton("Refresh", this);

    onlyHiddenCheckBox = new QCheckBox("Only hidden", this);
    onlyHiddenCheckBox->setChecked(true);

    statusLabel = new QLabel(this);

    topLayout->addWidget(refreshButton);
    topLayout->addWidget(onlyHiddenCheckBox);
    topLayout->addStretch();
    topLayout->addWidget(statusLabel);

    table = new QTableWidget(this);
    table->setColumnCount(5);

    table->setHorizontalHeaderLabels({
        "Subgroup",
        "Setting",
        "Hidden",
        "Subgroup GUID",
        "Setting GUID"
    });

    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);

    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    table->verticalHeader()->setVisible(false);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(table);

    setCentralWidget(central);

    connect(
        refreshButton,
        &QPushButton::clicked,
        this,
        &MainWindow::reloadTable
    );

    connect(
        onlyHiddenCheckBox,
        &QCheckBox::checkStateChanged,
        this,
        [this](Qt::CheckState) {
            reloadTable();
        }
    );
}

void MainWindow::reloadTable() {
    table->setRowCount(0);

    const int hiddenOnly = onlyHiddenCheckBox->isChecked() ? 1 : 0;

    int rc = enumerate_power_settings(
        hiddenOnly,
        MainWindow::enumerateCallback,
        this
    );

    if (rc != ERROR_SUCCESS) {
        QMessageBox::critical(
            this,
            "Error",
            QString("Failed to enumerate power settings. Error code: %1").arg(rc)
        );

        statusLabel->setText("Failed");
        return;
    }

    statusLabel->setText(
        QString("%1 item(s)").arg(table->rowCount())
    );
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

    const int row = table->rowCount();
    table->insertRow(row);

    auto *subgroupNameItem = new QTableWidgetItem(
        QString::fromWCharArray(item->subgroup_name)
    );

    auto *settingNameItem = new QTableWidgetItem(
        QString::fromWCharArray(item->setting_name)
    );

    auto *hiddenItem = new QTableWidgetItem(
        item->hidden ? "Yes" : "No"
    );

    auto *subgroupGuidItem = new QTableWidgetItem(
        QString::fromWCharArray(item->subgroup_guid_text)
    );

    auto *settingGuidItem = new QTableWidgetItem(
        QString::fromWCharArray(item->setting_guid_text)
    );

    table->setItem(row, 0, subgroupNameItem);
    table->setItem(row, 1, settingNameItem);
    table->setItem(row, 2, hiddenItem);
    table->setItem(row, 3, subgroupGuidItem);
    table->setItem(row, 4, settingGuidItem);
}
