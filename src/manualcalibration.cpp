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

static const double SUPPLY_100MV_VALUE = 0.1;
static const double SUPPLY_4_5V_VALUE = 4.5;
static const double SUPPLY_100MV_NEG_VALUE = -0.1;
static const double SUPPLY_4_5V_NEG_VALUE = -4.5;

/*Calibrations procedure stories*/
static const QStringList positiveOffsetStory = (QStringList() <<
					 R"(Calibrate the Positive Supply.
Measure the Voltage on the "V+" and
enter the value in the field below.
The value should be around 100mV)"
					 << R"(Calibrate the Positive Supply
Measure the Voltage on the "V+"
and enter the value in the field below.
The value should be around 4.5V)");
static const QStringList negativeOffsetStory = (QStringList() <<
					 R"(Calibrate the Negative Supply
Measure the Voltage on the "V-"
and enter the value in the field below.
The value should be around -100mV)"
					 << R"(Calibrate the Negative Supply
Measure the Voltage on the "V-"
and enter the value in the field below.
The value should be around -4.5V)");

ManualCalibration::ManualCalibration(struct iio_context *ctx, Filter *filt,
				     QPushButton *runButton, QJSEngine *engine,
				     ToolLauncher *parent, Calibration *cal) :
	Tool(ctx, runButton, nullptr, "Calibration", parent),
	ui(new Ui::ManualCalibration), filter(filt),
	eng(engine), calib(cal)
{
	ui->setupUi(this);
	calibListString << "Positive supply"
			<< "Negative supply"
			<< "Calibration parameters"
			<< "Autocalibration parameters";

	calibOption.insert("Positive supply", 0);
	calibOption.insert("Negative supply", 1);
	calibOption.insert("Calibration parameters", 2);
	calibOption.insert("Autocalibration parameters", 3);

	ui->calibList->addItems(calibListString);

	paramTable = new QTableWidget(ui->storyWidget);
	startParamTable = new QTableWidget(ui->storyWidget);
	paramTable->hide();
	startParamTable->hide();
	startParamTable->setEnabled(false);

	displayStartUpCalibrationValues();
	initParameters();

	TempUi = new Ui::CalibrationTemplate();
	TempWidget = new QWidget();
	TempUi->setupUi(TempWidget);

	TempUi->inputTableWidget->setRowCount(2);
	TempUi->inputTableWidget->setEnabled(false);

	connect(TempUi->nextButton, &QPushButton::clicked, this,
		&ManualCalibration::on_nextButton_clicked);
	connect(TempUi->restartButton, &QPushButton::clicked, this,
		&ManualCalibration::on_restartButton_clicked);
	connect(TempUi->finishButton, &QPushButton::clicked, this,
		&ManualCalibration::on_finishButton_clicked);

	setupPowerSupplyIio();
}

ManualCalibration::~ManualCalibration()
{

}

void ManualCalibration::startCalibration()
{
	qDebug() << "START: Calibration has started";

	switch (stCalibrationStory.calibProcedure) {
	case POSITIVE_OFFSET:
		positivePowerSupplyParam(stCalibrationStory.calibStep);
		break;

	case NEGATIVE_OFFSET:
		negativePowerSupplyParam(stCalibrationStory.calibStep);
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

	ui->storyWidget->show();
	ui->loadButton->setEnabled(false);
	ui->saveButton->setEnabled(false);

	switch (calibOption[temp]) {
	case POSITIVE_OFFSET:
		positivePowerSupplySetup();
		paramTable->hide();
		startParamTable->hide();
		TempWidget->show();
		break;
	case NEGATIVE_OFFSET:
		negativePowerSupplySetup();
		paramTable->hide();
		startParamTable->hide();
		TempWidget->show();
		break;
	case CALIB_PARAM:
		ui->storyWidget->layout()->addWidget(paramTable);
		paramTable->show();
		if(!ui->storyWidget->layout()->isEmpty())
			TempWidget->hide();
		startParamTable->hide();
		ui->loadButton->setEnabled(true);
		ui->saveButton->setEnabled(true);
		break;
	case START_CALIB_PARAM:
		ui->storyWidget->layout()->addWidget(startParamTable);
		startParamTable->show();
		if(!ui->storyWidget->layout()->isEmpty())
			TempWidget->hide();
		paramTable->hide();
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
	stCalibrationStory.storyName.append("Positive supply");
	ui->storyWidget->layout()->addWidget(TempWidget);
	TempUi->instructionText->setText(
		stCalibrationStory.story[stCalibrationStory.calibStep]);

	/*Set DAC to 0V*/
	setEnablePositiveSuppply(true);
	setPositiveValue(0.1);
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

	if (chan) {
		iio_channel_attr_write_bool(chan, "powerdown", false);
	}

	chan = iio_device_find_channel(dev3, "voltage1", false);

	if (chan) {
		iio_channel_attr_write_bool(chan, "powerdown", false);
	}

	/* ADF4360 globaal clock power down */
	iio_device_attr_write(dev3, "clk_powerdown", "0");
}

void ManualCalibration::positivePowerSupplyParam(const int step)
{
	double offset_Value;
	long long val = 0;
	double value = 0;

	qDebug() << "Set positive supply parameters";

	offset_Value = TempUi->lineEdit->text().toDouble();

	switch (step) {
	case STEP1:
		/*dac offset calibration*/
		stParameters.offset_pos_dac = SUPPLY_100MV_VALUE - offset_Value;
		qDebug() << "Positive offset DAC value: " << stParameters.offset_pos_dac;

		/*adc offset calibration*/
		iio_channel_attr_read_longlong(ch1r, "raw", &val);
		value = (double) val * 6.4 / 4095.0;
		stParameters.offset_pos_adc = offset_Value - value;
		qDebug() << "Positive offset ADC value: " << stParameters.offset_pos_adc;

		TempUi->inputTableWidget->setItem(0, 0, new QTableWidgetItem("100mV"));
		TempUi->inputTableWidget->setItem(0, 1, new QTableWidgetItem(QString::number(
				offset_Value) + QString("V")));
		TempUi->lineEdit->clear();

		paramTable->setItem(0, 1, new QTableWidgetItem(QString::number(
						stParameters.offset_pos_dac)));
		paramTable->setItem(2, 1, new QTableWidgetItem(QString::number(
						stParameters.offset_pos_adc)));
		paramTable->resizeColumnsToContents();

		/*Set dac to 4.5V*/
		setPositiveValue(4.5);
		break;

	case STEP2:

		/*dac gain calibration*/
		if (offset_Value != 0) {
			stParameters.gain_pos_dac = SUPPLY_4_5V_VALUE / (offset_Value +
						    stParameters.offset_pos_dac);
			qDebug() << "Positive gain DAC value: " << stParameters.gain_pos_dac;

			/*adc gain calibration*/
			iio_channel_attr_read_longlong(ch1r, "raw", &val);
			value = (double) val * 6.4 / 4095.0;
			stParameters.gain_pos_adc = offset_Value / (value +
						    stParameters.offset_pos_adc);
			qDebug() << "Positive gain ADC value: " << stParameters.gain_pos_adc;
		}

		TempUi->restartButton->setVisible(true);
		TempUi->inputTableWidget->setItem(1, 0, new QTableWidgetItem("4.5V"));
		TempUi->inputTableWidget->setItem(1, 1, new QTableWidgetItem(QString::number(
				offset_Value) + QString("V")));

		paramTable->setItem(1, 1, new QTableWidgetItem(QString::number(
						stParameters.gain_pos_dac)));
		paramTable->setItem(3, 1, new QTableWidgetItem(QString::number(
						stParameters.gain_pos_adc)));
		paramTable->resizeColumnsToContents();

		TempUi->nextButton->setVisible(false);
		TempUi->finishButton->setVisible(true);
		break;
	}

	TempUi->lineEdit->clear();
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

void ManualCalibration::negativePowerSupplySetup()
{
	TempUi->restartButton->setVisible(false);
	TempUi->nextButton->setVisible(true);
	TempUi->finishButton->setVisible(false);

	/*Initialize the tab*/
	stCalibrationStory.calibProcedure = NEGATIVE_OFFSET;
	stCalibrationStory.calibStep = 0;
	stCalibrationStory.story = negativeOffsetStory;
	stCalibrationStory.storyName.clear();
	stCalibrationStory.storyName.append("Negative supply");
	ui->storyWidget->layout()->addWidget(TempWidget);
	TempUi->instructionText->setText(
		stCalibrationStory.story[stCalibrationStory.calibStep]);

	setEnableNegativeSuppply(true);
	setNegativeValue(-0.1);
}

void ManualCalibration::negativePowerSupplyParam(const int step)
{
	double offset_Value;
	long long val = 0;
	double value = 0;

	qDebug() << "Set negative supply parameters";

	offset_Value = TempUi->lineEdit->text().toDouble();

	switch (step) {
	case STEP1:
		/*dac offset calibration*/
		stParameters.offset_neg_dac = SUPPLY_100MV_NEG_VALUE - offset_Value;
		qDebug() << "Negative offset DAC value: " << stParameters.offset_neg_dac;

		/*adc offset calibration*/
		iio_channel_attr_read_longlong(ch2r, "raw", &val);
		value = (double) val * (-6.4) / 4095.0;
		stParameters.offset_neg_adc = offset_Value - value;
		qDebug() << "Negative offset ADC value: " << stParameters.offset_neg_adc;

		TempUi->inputTableWidget->setItem(0, 0, new QTableWidgetItem("-100mV"));
		TempUi->inputTableWidget->setItem(0, 1, new QTableWidgetItem(QString::number(
				offset_Value) + QString("V")));
		TempUi->lineEdit->clear();

		paramTable->setItem(4, 1, new QTableWidgetItem(QString::number(
						stParameters.offset_neg_dac)));
		paramTable->setItem(6, 1, new QTableWidgetItem(QString::number(
						stParameters.offset_neg_adc)));
		paramTable->resizeColumnsToContents();

		/*Set dac to -4.5V*/
		setNegativeValue(-4.5);
		break;

	case STEP2:

		/*dac gain calibration*/
		if (offset_Value != 0) {
			stParameters.gain_neg_dac = SUPPLY_4_5V_NEG_VALUE / (offset_Value +
						    stParameters.offset_neg_dac);
			qDebug() << "Negative gain DAC value: " << stParameters.gain_neg_dac;

			/*adc gain calibration*/
			iio_channel_attr_read_longlong(ch2r, "raw", &val);
			value = (double) val * (-6.4) / 4095.0;
			stParameters.gain_neg_adc =  offset_Value / (value +
						     stParameters.offset_neg_adc);
			qDebug() << "Negative gain ADC value: " << stParameters.gain_neg_adc;
		}

		TempUi->restartButton->setVisible(true);
		TempUi->inputTableWidget->setItem(1, 0, new QTableWidgetItem("4.5V"));
		TempUi->inputTableWidget->setItem(1, 1, new QTableWidgetItem(QString::number(
				offset_Value) + QString("V")));

		paramTable->setItem(5, 1, new QTableWidgetItem(QString::number(
						stParameters.gain_neg_dac)));
		paramTable->setItem(7, 1, new QTableWidgetItem(QString::number(
						stParameters.gain_neg_adc)));
		paramTable->resizeColumnsToContents();

		TempUi->nextButton->setVisible(false);
		TempUi->finishButton->setVisible(true);
		break;
	}

	TempUi->lineEdit->clear();
}

void ManualCalibration::setEnableNegativeSuppply(bool enabled)
{
	iio_channel_attr_write_bool(ch2w, "powerdown", !enabled);

	if (pd_neg) {
		iio_channel_attr_write_bool(pd_neg, "user_supply_powerdown", !enabled);
	} else {
		iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", !enabled);
	}
}

void ManualCalibration::setNegativeValue(double value)
{
	long long val = value * 4095.0 / (-5.1 * 1.2);

	iio_channel_attr_write_longlong(ch2w, "raw", val);
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
	QStringList tableHeader;

	tableHeader <<"Name"<<"Value";
	startParamTable->setRowCount(8);
	startParamTable->setColumnCount(2);
	startParamTable->setHorizontalHeaderLabels(tableHeader);

	startParamTable->setItem(0, 0, new QTableWidgetItem("ADC offset Ch0"));
	startParamTable->setItem(0, 1, new QTableWidgetItem(QString::number(
				calib->adcOffsetChannel0())));

	startParamTable->setItem(1, 0, new QTableWidgetItem("ADC offset Ch1"));
	startParamTable->setItem(1, 1, new QTableWidgetItem(QString::number(
				calib->adcOffsetChannel1())));

	startParamTable->setItem(2, 0, new QTableWidgetItem("ADC gain Ch0"));
	startParamTable->setItem(2, 1, new QTableWidgetItem(QString::number(
				calib->adcGainChannel0())));

	startParamTable->setItem(3, 0, new QTableWidgetItem("ADC gain Ch1"));
	startParamTable->setItem(3, 1, new QTableWidgetItem(QString::number(
				calib->adcGainChannel1())));

	startParamTable->setItem(4, 0, new QTableWidgetItem("DAC A offset"));
	startParamTable->setItem(4, 1, new QTableWidgetItem(QString::number(
				calib->dacAoffset())));

	startParamTable->setItem(5, 0, new QTableWidgetItem("DAC B offset"));
	startParamTable->setItem(5, 1, new QTableWidgetItem(QString::number(
				calib->dacBoffset())));

	startParamTable->setItem(6, 0, new QTableWidgetItem("DAC A vlsb"));
	startParamTable->setItem(6, 1, new QTableWidgetItem(QString::number(calib->dacAvlsb())));

	startParamTable->setItem(7, 0, new QTableWidgetItem("DAC B vlsb"));
	startParamTable->setItem(7, 1, new QTableWidgetItem(QString::number(calib->dacBvlsb())));

	startParamTable->resizeColumnsToContents();
}

void ManualCalibration::initParameters(void)
{
	QStringList tableHeader;
	const char *name;
	const char *value;
	QTableWidgetItem *item;

	tableHeader <<"Name"<<"Value";
	paramTable->setRowCount(8);
	paramTable->setColumnCount(2);
	paramTable->setHorizontalHeaderLabels(tableHeader);

	for (int i = 4; i < 12; i++) {
		if (!iio_context_get_attr(ctx, i, &name, &value)) {
			item = new QTableWidgetItem(QString(name + 4));
			item->setFlags(Qt::ItemIsSelectable);
			paramTable->setItem(i - 4, 0, item);
			paramTable->setItem(i - 4, 1, new QTableWidgetItem(QString(value)));
		}
	}

	paramTable->resizeColumnsToContents();
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

		for (int i = 0; i < paramTable->rowCount(); i++)
			stream << "cal," << paramTable->item(i,0)->text() << "="
			       << paramTable->item(i,1)->text() << endl;
	}

	file.close();
}

void ManualCalibration::on_restartButton_clicked()
{
	switch (stCalibrationStory.calibProcedure) {
	case POSITIVE_OFFSET:
		positivePowerSupplySetup();
		break;

	case NEGATIVE_OFFSET:
		negativePowerSupplySetup();
		break;
	}

	TempUi->inputTableWidget->clearContents();
}

void ManualCalibration::on_finishButton_clicked()
{
	ui->storyWidget->layout()->removeWidget(TempWidget);

	QSizePolicy sp_retain = ui->storyWidget->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->storyWidget->setSizePolicy(sp_retain);

	ui->storyWidget->hide();
	updateParameters();

	TempUi->inputTableWidget->clearContents();
	setNegativeValue(0);
	setPositiveValue(0);
	setEnablePositiveSuppply(false);
	setEnableNegativeSuppply(false);
}



void ManualCalibration::on_autoButton_clicked()
{
	if (calib->isInitialized()) {
		calib->setHardwareInCalibMode();
		calib->calibrateAll();
		calib->restoreHardwareFromCalibMode();
	}
	displayStartUpCalibrationValues();
}
