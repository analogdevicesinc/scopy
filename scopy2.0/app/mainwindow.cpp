#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <scopy/gui/two_buttons_widget.hpp>
#include <scopy/gui/run_single_widget.hpp>
#include <scopy/gui/subsection_separator.hpp>
#include <scopy/gui/settings_pair_widget.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->centralLayout->addWidget(new scopy::gui::RunSingleWidget);
  ui->vLayout->addWidget(new scopy::gui::SubsectionSeparator("My label", true));
  ui->vLayout->addWidget(new scopy::gui::SubsectionSeparator("Another label", false));
  ui->vLayout->addWidget(new scopy::gui::SettingsPairWidget);
}

MainWindow::~MainWindow() { delete ui; }
