#include "widgets/measurementpanel.h"
#include "ui_measure_panel.h"
#include <QWidget>

using namespace scopy::gui;

MeasurementPanel::MeasurementPanel(QWidget *parent) : QWidget(parent)
{
	ui = new Ui::MeasurementsPanel();
	ui->setupUi(this);

	connect(ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged,
			ui->scrollArea_2->horizontalScrollBar(), &QScrollBar::setRange);

	connect(ui->scrollArea_2->horizontalScrollBar(), &QScrollBar::valueChanged,
			ui->scrollArea->horizontalScrollBar(), &QScrollBar::setValue);
	connect(ui->scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged,
			ui->scrollArea_2->horizontalScrollBar(), &QScrollBar::setValue);

	connect(ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged,
			[=](double v1, double v2){
				ui->scrollArea_2->widget()->setFixedWidth(ui->scrollAreaWidgetContents->width());
			});

}

void MeasurementPanel::addMeasurement(MeasurementLabel *meas) {
	ui->verticalLayout_2->addWidget(meas);
}

void MeasurementPanel::removeMeasurement(MeasurementLabel *meas) {
	ui->verticalLayout_2->removeWidget(meas);
}

void MeasurementPanel::update() {

}

QWidget *MeasurementPanel::cursorArea()
{
	return ui->cursorReadouts;
}


