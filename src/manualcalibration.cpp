/*
 * Copyright 2017 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "manualcalibration.h"
#include "ui_manualcalibration.h"
#include "ui_calibratetemplate.h"
#include <QDebug>

using namespace adiscope;

ManualCalibration::ManualCalibration(struct iio_context *ctx, Filter *filt,
				     QPushButton *runButton, QJSEngine *engine,
				     ToolLauncher *parent, Calibration *cal) :
	Tool(ctx, runButton, nullptr, "Calibration", parent),
	ui(new Ui::ManualCalibration), filter(filt),
	eng(engine), calib(cal)
{
	ui->setupUi(this);
	calibListString << "Positive offset"
			<< "Negative offset";

	calibOption.insert("Positive offset", 0);
	calibOption.insert("Negative offset", 1);

	ui->calibList->addItems(calibListString);

	displayStartUpCalibrationValues();

}

ManualCalibration::~ManualCalibration()
{

}

int ManualCalibration::startCalibration()
{
	qDebug() << "START: Calibration has started";

	switch(stCalibrationStory.calibProcedure){
	case POSITIVE_OFFSET:
		positivePowerSupplyParam(stCalibrationStory.calibStep);
		break;

	case NEGATIVE_OFFSET:
		break;
	}
	//go to next step
	nextStep();
}

void ManualCalibration::nextStep()
{
	if (stCalibrationStory.story.count() > (stCalibrationStory.calibStep + 1)) {
		stCalibrationStory.calibStep++;
		positiveTempUi->instructionText->setText(stCalibrationStory.story[stCalibrationStory.calibStep]);
	} else {
		ui->tabWidget->removeTab(3); //remove the story tab
		qDebug() << "Calibration procedure finished";
	}

}
void ManualCalibration::on_calibList_itemClicked(QListWidgetItem *item)
{
	qDebug() << "Calibration list item clicked" << item->text().toLocal8Bit();
	QString temp = item->text();

	ui->tabWidget->removeTab(3); //remove the story tab

	switch(calibOption[temp]) {
	case POSITIVE_OFFSET:
		positivePowerSupplySetup();
		break;
	case NEGATIVE_OFFSET:
		break;
	}
}

void ManualCalibration::positivePowerSupplySetup()
{
	positiveTempUi = new Ui::CalibrationTemplate();
	QWidget *positiveTemp = new QWidget();
	positiveTempUi->setupUi(positiveTemp);

	connect(positiveTempUi->nextButton, &QPushButton::clicked, this, &ManualCalibration::on_nextButton_clicked);

	/*Initialize the tab*/
	stCalibrationStory.calibProcedure = POSITIVE_OFFSET;
	stCalibrationStory.calibStep = 0;
	stCalibrationStory.story = positiveOffsetStory;
	stCalibrationStory.storyName.clear();
	stCalibrationStory.storyName.append("Positive offset");
	ui->tabWidget->addTab(positiveTemp, stCalibrationStory.storyName);
	positiveTempUi->instructionText->setText(stCalibrationStory.story[stCalibrationStory.calibStep]);

	/*For power supply calibration*/
	struct iio_device *dev1 = iio_context_find_device(ctx, "ad5627");
	struct iio_device *dev2 = iio_context_find_device(ctx, "ad9963");
	struct iio_device *dev3 = iio_context_find_device(ctx, "m2k-fabric");

	if (!dev1 || !dev2 || !dev3)
		throw std::runtime_error("Unable to find device\n");

	this->ch1w = iio_device_find_channel(dev1, "voltage0", true);
	this->ch2w = iio_device_find_channel(dev1, "voltage1", true);
	this->ch1r = iio_device_find_channel(dev2, "voltage2", false);
	this->ch2r = iio_device_find_channel(dev2, "voltage1", false);
	this->pd_pos = iio_device_find_channel(dev3, "voltage2", true);
	this->pd_neg = iio_device_find_channel(dev3, "voltage3", true); /* For HW Rev. >= C */

	if (!ch1w || !ch2w || !ch1r || !ch2r || !pd_pos)
		throw std::runtime_error("Unable to find channels\n");

	/*Set DAC to 0V*/
	setEnablePositiveSuppply(true);
	setPositiveValue(0);
}

void ManualCalibration::positivePowerSupplyParam(const int& step)
{
	double offset_Value;

	qDebug() << "Set positive offset parameters";

	switch(step) {
	case STEP1:
		offset_Value = positiveTempUi->lineEdit->text().toDouble();
		stParameters.positiveOffset = SUPPLY_0V_VALUE - offset_Value;
		positiveTempUi->lineEdit->clear();
		break;
	case STEP2:
		offset_Value = positiveTempUi->lineEdit->text().toDouble();
		stParameters.positiveOffset = (stParameters.positiveOffset + (SUPPLY_4_5V_VALUE - offset_Value)) / 2;
		qDebug() << "Positive offset value: " << stParameters.positiveOffset;
		break;
	case STEP3:
		break;
	}
}

void ManualCalibration::setEnablePositiveSuppply(bool enabled)
{
	iio_channel_attr_write_bool(ch1w, "powerdown", enabled);
	iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", enabled);
}

void ManualCalibration::setPositiveValue(double value)
{
	long long val = value * 4095.0 / (5.02 * 1.2);

	iio_channel_attr_write_longlong(ch1w, "raw", val);
}

void ManualCalibration::on_nextButton_clicked()
{
    startCalibration();
}

void ManualCalibration::setCalibration(Calibration *cal)
{
	calib = cal;
}

void ManualCalibration::displayStartUpCalibrationValues(void)
{
	QTableWidget *table = new QTableWidget(this);
	QStringList tableHeader;

	tableHeader <<"Name"<<"Text";
	table->setRowCount(8);
	table->setColumnCount(2);
	table->setHorizontalHeaderLabels(tableHeader);

	table->setItem(0, 0, new QTableWidgetItem("ADC offset Ch0"));
	table->setItem(0, 1, new QTableWidgetItem(QString::number(calib->adcOffsetChannel0())));

	table->setItem(1, 0, new QTableWidgetItem("ADC offset Ch1"));
	table->setItem(1, 1, new QTableWidgetItem(QString::number(calib->adcOffsetChannel1())));

	table->setItem(2, 0, new QTableWidgetItem("ADC gain Ch0"));
	table->setItem(2, 1, new QTableWidgetItem(QString::number(calib->adcGainChannel0())));

	table->setItem(3, 0, new QTableWidgetItem("ADC gain Ch0"));
	table->setItem(3, 1, new QTableWidgetItem(QString::number(calib->adcGainChannel1())));

	table->setItem(4, 0, new QTableWidgetItem("DAC A offset"));
	table->setItem(4, 1, new QTableWidgetItem(QString::number(calib->dacAoffset())));

	table->setItem(5, 0, new QTableWidgetItem("DAC B offset"));
	table->setItem(5, 1, new QTableWidgetItem(QString::number(calib->dacBoffset())));

	table->setItem(6, 0, new QTableWidgetItem("DAC A vlsb"));
	table->setItem(6, 1, new QTableWidgetItem(QString::number(calib->dacAvlsb())));

	table->setItem(7, 0, new QTableWidgetItem("DAC B vlsb"));
	table->setItem(7, 1, new QTableWidgetItem(QString::number(calib->dacBvlsb())));

	ui->tabWidget->addTab(table, "Startup calibration");

	//calib->adcOffsetChannel0();
}
