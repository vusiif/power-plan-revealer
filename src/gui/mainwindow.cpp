#include "mainwindow.h"

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
