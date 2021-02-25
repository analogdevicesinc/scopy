#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <scopy/gui/twobuttonswidget.h>
//#include <twobuttonswidget.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->centralLayout->addWidget(new scopy::gui::TwoButtonsWidget("hello"));
}

MainWindow::~MainWindow() { delete ui; }
