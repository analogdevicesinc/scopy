#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <scopy/gui/two_buttons_widget.hpp>
#include <scopy/gui/run_single_widget.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->centralLayout->addWidget(new scopy::gui::RunSingleWidget);
}

MainWindow::~MainWindow() { delete ui; }
