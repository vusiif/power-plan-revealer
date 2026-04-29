#include "mainwindow.h"

#include "power_attr.h"

#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QString>
#include <QTreeWidgetItem>

#include <string>

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

    const QString resultText = hidden
    ? "设置项已经隐藏。"
    : "设置项已经取消隐藏。";

    const QString beforeText = QString::number(beforeAttributes, 16)
        .toUpper()
        .rightJustified(8, '0');

    const QString afterText = QString::number(afterAttributes, 16)
        .toUpper()
        .rightJustified(8, '0');

    const QString noteText =
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
            resultText,
            settingName,
            beforeText,
            afterText,
            noteText
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
