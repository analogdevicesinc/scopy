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
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QTime>

using namespace adiscope;

ManualCalibration::ManualCalibration(struct iio_context *ctx, Filter *filt,
				     QPushButton *runButton, QJSEngine *engine,
				     ToolLauncher *parent, Calibration *cal) :
	Tool(ctx, runButton, nullptr, "Calibration", parent),
	ui(new Ui::ManualCalibration), filter(filt),
	eng(engine), calib(cal)
{
	ui->setupUi(this);
	calibListString << "Positive supply"
			<< "Negative supply";

	calibOption.insert("Positive supply", 0);
	calibOption.insert("Negative supply", 1);

	ui->calibList->addItems(calibListString);

	displayStartUpCalibrationValues();
	initParameters();

	TempUi = new Ui::CalibrationTemplate();
	TempWidget = new QWidget();
	TempUi->setupUi(TempWidget);

	TempUi->inputTableWidget->setRowCount(2);

	connect(TempUi->nextButton, &QPushButton::clicked, this,
		&ManualCalibration::on_nextButton_clicked);
	connect(TempUi->restartButton, &QPushButton::clicked, this,
		&ManualCalibration::on_restartButton_clicked);
	connect(TempUi->finishButton, &QPushButton::clicked, this,
		&ManualCalibration::on_finishButton_clicked);
}

ManualCalibration::~ManualCalibration()
{

}

int ManualCalibration::startCalibration()
{
	qDebug() << "START: Calibration has started";

	switch (stCalibrationStory.calibProcedure) {
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
		TempUi->instructionText->setText(
			stCalibrationStory.story[stCalibrationStory.calibStep]);
	} else {
		qDebug() << "Calibration procedure finished";
	}
}
void ManualCalibration::on_calibList_itemClicked(QListWidgetItem *item)
{
	qDebug() << "Calibration list item clicked" << item->text().toLocal8Bit();
	QString temp = item->text();

	ui->tabWidget->removeTab(3); //remove the story tab

	switch (calibOption[temp]) {
	case POSITIVE_OFFSET:
		positivePowerSupplySetup();
		break;

	case NEGATIVE_OFFSET:
		break;
	}
}

void ManualCalibration::positivePowerSupplySetup()
{
	TempUi->restartButton->setVisible(false);
	TempUi->nextButton->setVisible(true);
	TempUi->finishButton->setVisible(false);

	/*Initialize the tab*/
	stCalibrationStory.calibProcedure = POSITIVE_OFFSET;
	stCalibrationStory.calibStep = 0;
	stCalibrationStory.story = positiveOffsetStory;
	stCalibrationStory.storyName.clear();
	stCalibrationStory.storyName.append("Positive offset");
	ui->tabWidget->addTab(TempWidget, stCalibrationStory.storyName);
	TempUi->instructionText->setText(
		stCalibrationStory.story[stCalibrationStory.calibStep]);

	setupPowerSupplyIio();

	/*Set DAC to 0V*/
	setEnablePositiveSuppply(true);
	setPositiveValue(0);
}

void ManualCalibration::setupPowerSupplyIio()
{
	/*For power supply calibration*/
	struct iio_device *dev1 = iio_context_find_device(ctx, "ad5627");
	struct iio_device *dev2 = iio_context_find_device(ctx, "ad9963");
	struct iio_device *dev3 = iio_context_find_device(ctx, "m2k-fabric");

	if (!dev1 || !dev2 || !dev3) {
		throw std::runtime_error("Unable to find device\n");
	}

	this->ch1w = iio_device_find_channel(dev1, "voltage0", true);
	this->ch2w = iio_device_find_channel(dev1, "voltage1", true);
	this->ch1r = iio_device_find_channel(dev2, "voltage2", false);
	this->ch2r = iio_device_find_channel(dev2, "voltage1", false);
	this->pd_pos = iio_device_find_channel(dev3, "voltage2", true);
	this->pd_neg = iio_device_find_channel(dev3, "voltage3",
					       true); /* For HW Rev. >= C */

	if (!ch1w || !ch2w || !ch1r || !ch2r || !pd_pos) {
		throw std::runtime_error("Unable to find channels\n");
	}

	/*enable ADCs*/
	struct iio_channel *chan;
	/* These are the two ADC amplifiers */
	chan = iio_device_find_channel(dev3, "voltage0", false);
	if (chan)
		iio_channel_attr_write_bool(chan, "powerdown", false);

	chan = iio_device_find_channel(dev3, "voltage1", false);
	if (chan)
		iio_channel_attr_write_bool(chan, "powerdown", false);

	/* ADF4360 globaal clock power down */
	iio_device_attr_write(dev3, "clk_powerdown", "0");
}

void ManualCalibration::positivePowerSupplyParam(const int& step)
{
	double offset_Value;
	long long val = 0;
	double value = 0;

	qDebug() << "Set positive offset parameters";

	offset_Value = TempUi->lineEdit->text().toDouble();

	switch (step) {
	case STEP1:
		/*Set dac to 100mV*/
		setPositiveValue(0.1);

		/*dac offset calibration*/
		stParameters.offset_pos_dac = SUPPLY_100MV_VALUE - offset_Value;
		qDebug() << "Positive offset DAC value: " << stParameters.offset_pos_dac;

		/*adc offset calibration*/
		iio_channel_attr_read_longlong(ch1r, "raw", &val);
		value = (double) val * 6.4 / 4095.0;
		stParameters.offset_pos_adc = value - offset_Value;
		qDebug() << "Positive offset ADC value: " << stParameters.offset_pos_adc;

		TempUi->inputTableWidget->setItem(0, 0, new QTableWidgetItem("100mV"));
		TempUi->inputTableWidget->setItem(0, 1, new QTableWidgetItem(QString::number(
					offset_Value) + QString("V")));
		TempUi->lineEdit->clear();

		ui->paramTable->setItem(0, 1, new QTableWidgetItem(QString::number(
									   stParameters.offset_pos_dac)));
		ui->paramTable->setItem(2, 1, new QTableWidgetItem(QString::number(
									   stParameters.offset_pos_adc)));
		ui->paramTable->resizeColumnsToContents();
		break;

	case STEP2:
		/*Set dac to 4.5V*/
		setPositiveValue(4.5);

		/*dac gain calibration*/
		if (offset_Value != 0){
			stParameters.gain_pos_dac = SUPPLY_4_5V_VALUE / offset_Value;
			qDebug() << "Positive gain DAC value: " << stParameters.gain_pos_dac;

			/*adc gain calibration*/
			iio_channel_attr_read_longlong(ch1r, "raw", &val);
			value = (double) val * 6.4 / 4095.0;
			stParameters.gain_pos_adc = value / offset_Value;
			qDebug() << "Positive gain ADC value: " << stParameters.gain_pos_adc;
		}
		TempUi->restartButton->setVisible(true);
		TempUi->inputTableWidget->setItem(1, 0, new QTableWidgetItem("4.5V"));
		TempUi->inputTableWidget->setItem(1, 1, new QTableWidgetItem(QString::number(
					offset_Value) + QString("V")));

		ui->paramTable->setItem(1, 1, new QTableWidgetItem(QString::number(
									   stParameters.gain_pos_dac)));
		ui->paramTable->setItem(3, 1, new QTableWidgetItem(QString::number(
									   stParameters.gain_pos_adc)));
		ui->paramTable->resizeColumnsToContents();

		TempUi->nextButton->setVisible(false);
		TempUi->finishButton->setVisible(true);
		break;
	}
}


void ManualCalibration::setEnablePositiveSuppply(bool enabled)
{
	iio_channel_attr_write_bool(ch1w, "powerdown", !enabled);
	iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", !enabled);
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

	tableHeader <<"Name"<<"Value";
	table->setRowCount(8);
	table->setColumnCount(2);
	table->setHorizontalHeaderLabels(tableHeader);

	table->setItem(0, 0, new QTableWidgetItem("ADC offset Ch0"));
	table->setItem(0, 1, new QTableWidgetItem(QString::number(
				calib->adcOffsetChannel0())));

	table->setItem(1, 0, new QTableWidgetItem("ADC offset Ch1"));
	table->setItem(1, 1, new QTableWidgetItem(QString::number(
				calib->adcOffsetChannel1())));

	table->setItem(2, 0, new QTableWidgetItem("ADC gain Ch0"));
	table->setItem(2, 1, new QTableWidgetItem(QString::number(
				calib->adcGainChannel0())));

	table->setItem(3, 0, new QTableWidgetItem("ADC gain Ch0"));
	table->setItem(3, 1, new QTableWidgetItem(QString::number(
				calib->adcGainChannel1())));

	table->setItem(4, 0, new QTableWidgetItem("DAC A offset"));
	table->setItem(4, 1, new QTableWidgetItem(QString::number(
				calib->dacAoffset())));

	table->setItem(5, 0, new QTableWidgetItem("DAC B offset"));
	table->setItem(5, 1, new QTableWidgetItem(QString::number(
				calib->dacBoffset())));

	table->setItem(6, 0, new QTableWidgetItem("DAC A vlsb"));
	table->setItem(6, 1, new QTableWidgetItem(QString::number(calib->dacAvlsb())));

	table->setItem(7, 0, new QTableWidgetItem("DAC B vlsb"));
	table->setItem(7, 1, new QTableWidgetItem(QString::number(calib->dacBvlsb())));

	table->resizeColumnsToContents();

	ui->tabWidget->addTab(table, "Startup calibration");
}

void ManualCalibration::initParameters(void)
{
	QTableWidget *table = ui->paramTable;
	QStringList tableHeader;
	const char *name;
	const char *value;

	tableHeader <<"Name"<<"Value";
	table->setRowCount(8);
	table->setColumnCount(2);
	table->setHorizontalHeaderLabels(tableHeader);

	for (int i = 4; i < 12; i++) {
		if (!iio_context_get_attr(ctx, i, &name, &value)) {
			table->setItem(i - 4, 0, new QTableWidgetItem(QString(name + 4)));
			table->setItem(i - 4, 1, new QTableWidgetItem(QString(value)));
		}
	}

	table->resizeColumnsToContents();
}

void ManualCalibration::updateParameters(void)
{

}

void ManualCalibration::on_loadButton_clicked()
{
	initParameters();
}

void ManualCalibration::on_saveButton_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Save File"),
			   "/home",
			   tr("ini (*.ini)"));
	QFile file(fileName);
	QString temp_ad9963 = QString::number(calib->getIioDevTemp(QString("ad9963")));
	QString temp_fpga = QString::number(calib->getIioDevTemp(QString("xadc")));

	if (file.open(QFile::WriteOnly | QFile::Truncate)) {
		QTextStream stream(&file);

		stream << "#Calibration time: " << QDate::currentDate().toString() << ", "
		       << QTime::currentTime().toString()
		       << "\n#ad9963 temperature: " << temp_ad9963
		       << tr(" °C") << "\n#FPGA temperature: "<< temp_fpga
		       << tr(" °C") << endl;
		for (int i = 0; i < ui->paramTable->rowCount(); i++)
			stream << "cal," << ui->paramTable->item(i,0)->text() << "="
			       << ui->paramTable->item(i,1)->text() << endl;
	}
	file.close();
}

void ManualCalibration::on_restartButton_clicked()
{
	ui->tabWidget->removeTab(3); //remove the story tab
	positivePowerSupplySetup();
	ui->tabWidget->setCurrentIndex(3);
}

void ManualCalibration::on_finishButton_clicked()
{
	ui->tabWidget->removeTab(3); //remove the story tab
	updateParameters();
}
