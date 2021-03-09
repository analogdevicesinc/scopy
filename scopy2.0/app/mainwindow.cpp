#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include <scopy/gui/run_single_widget.hpp>
#include <scopy/gui/settings_pair_widget.hpp>
#include <scopy/gui/subsection_separator.hpp>
#include <scopy/gui/two_buttons_widget.hpp>
#include <scopy/gui/spinbox_a.hpp>
#include <scopy/gui/custom_menu_button.hpp>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->centralLayout->addWidget(new scopy::gui::RunSingleWidget);
	ui->vLayout->addWidget(new scopy::gui::SubsectionSeparator("My label", true));
	ui->vLayout->addWidget(new scopy::gui::SubsectionSeparator("Another label", false));
	ui->vLayout->addWidget(new scopy::gui::SettingsPairWidget);

	scopy::gui::ScaleSpinButton* scaleSpinBox = new scopy::gui::ScaleSpinButton({{"sps", 1E0}, {"ksps", 1E+3}, {"Msps", 1E+6}},
							    tr("Sample Rate"), 1, 10e7, true, false, ui->centralwidget, {1, 2, 5});

	ui->vLayout->addWidget(scaleSpinBox);
	ui->vLayout->addWidget(new scopy::gui::CustomMenuButton(ui->centralwidget, "Curors", true));
	ui->vLayout->addWidget(new scopy::gui::CustomMenuButton(ui->centralwidget, "Trigger", false));
}

MainWindow::~MainWindow() { delete ui; }
