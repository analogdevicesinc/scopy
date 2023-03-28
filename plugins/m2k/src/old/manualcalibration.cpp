/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "manualcalibration.h"
#include "ui_manualcalibration.h"
#include "ui_calibratetemplate.h"
#include "manual_calibration_api.hpp"

#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QTime>

/* libm2k includes */
#include <libm2k/contextbuilder.hpp>
#include <libm2k/m2k.hpp>
#include <libm2k/m2kexceptions.hpp>
#include <libm2k/analog/m2kpowersupply.hpp>
#include <libm2k/analog/dmm.hpp>
#include "m2kpluginExceptionHandler.h"
#include <pluginbase/scopyjs.h>

#include <QLoggingCategory>
Q_LOGGING_CATEGORY(CAT_M2K_CALIBRATION_MANUAL,"M2kCalibrationManual")

using namespace adiscope::m2k;
using namespace libm2k::context;
using namespace libm2k::analog;

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
Measure the Voltage on the "V+" and
enter the value in the field below.
The value should be around 4.5V)");
static const QStringList negativeOffsetStory = (QStringList() <<
					 R"(Calibrate the Negative Supply
Measure the Voltage on the "V-" and
enter the value in the field below.
The value should be around -100mV)"
					 << R"(Calibrate the Negative Supply
Measure the Voltage on the "V-" and
enter the value in the field below.
The value should be around -4.5V)");

ManualCalibration::ManualCalibration(struct iio_context *ctx, Filter *filt,
				     ToolMenuEntry *tme,
				     QWidget *parent, Calibration *cal) :
	M2kTool(ctx, tme, new ManualCalibration_API(this), "Calibration", parent),
	ui(new Ui::ManualCalibration), filter(filt), ctx(ctx),
	calib(cal),
	calibrationFilePath(""),
	m_m2k_context(m2kOpen(ctx, "")),
	m_m2k_powersupply(m_m2k_context->getPowerSupply())
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
		&ManualCalibration::nextButton_clicked);
	connect(TempUi->restartButton, &QPushButton::clicked, this,
		&ManualCalibration::restartButton_clicked);
	connect(TempUi->finishButton, &QPushButton::clicked, this,
		&ManualCalibration::finishButton_clicked);

	ui->calibList->setCurrentRow(3); //set to autocalibration parameters
	on_calibList_itemClicked(ui->calibList->currentItem());

	m_dmm_ad9963 = m_m2k_context->getDMM("ad9963");
	m_dmm_xadc = m_m2k_context->getDMM("xadc");

	eng = ScopyJS::GetInstance()->engine();
}

ManualCalibration::~ManualCalibration()
{
	delete TempWidget;
	delete TempUi;
	delete api;
	delete ui;
}

void ManualCalibration::startCalibration()
{
	qDebug(CAT_M2K_CALIBRATION_MANUAL) << "START: Calibration has started";

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
		qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Calibration procedure finished";
	}
}
void ManualCalibration::on_calibList_itemClicked(QListWidgetItem *item)
{
	qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Calibration list item clicked" << item->text().toLocal8Bit();
	QString temp = item->text();

	ui->storyWidget->show();
	ui->loadButton->setEnabled(false);
	ui->saveButton->setEnabled(false);

	// Clean up the current calibration story
	stCalibrationStory.calibProcedure = -1;
	stCalibrationStory.calibStep = -1;
	stCalibrationStory.story = QStringList();
	stCalibrationStory.storyName.clear();

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

void ManualCalibration::positivePowerSupplyParam(const int step)
{
	double offset_Value;
	double value = 0;

	qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Set positive supply parameters";

	offset_Value = TempUi->lineEdit->text().toDouble();

	switch (step) {
	case STEP1:
		/*dac offset calibration*/
		stParameters.offset_pos_dac = SUPPLY_100MV_VALUE - offset_Value;
		qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Positive offset DAC value: " << stParameters.offset_pos_dac;

		/*adc offset calibration*/
		try {
			value = m_m2k_powersupply->readChannel(0, false);
		} catch (libm2k::m2k_exception &e) {
			HANDLE_EXCEPTION(e)
			qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Can't read value: " << e.what();
		}

		stParameters.offset_pos_adc = offset_Value - value;
		qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Positive offset ADC value: " << stParameters.offset_pos_adc;

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
			qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Positive gain DAC value: " << stParameters.gain_pos_dac;

			/*adc gain calibration*/
			try {
				value = m_m2k_powersupply->readChannel(0, false);
			} catch (libm2k::m2k_exception &e) {
				HANDLE_EXCEPTION(e)
				qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Can't read value: " << e.what();
			}
			stParameters.gain_pos_adc = offset_Value / (value +
						    stParameters.offset_pos_adc);
			qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Positive gain ADC value: " << stParameters.gain_pos_adc;
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
	try {
		m_m2k_powersupply->enableChannel(0, enabled);
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e)
		qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Can't enable channel: " << e.what();
	}
}

void ManualCalibration::setPositiveValue(double value)
{
	try {
		m_m2k_powersupply->pushChannel(0, value, false);
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e)
		qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Can't write value: " << e.what();
	}
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
	double value = 0;

	qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Set negative supply parameters";

	offset_Value = TempUi->lineEdit->text().toDouble();

	switch (step) {
	case STEP1:
		/*dac offset calibration*/
		stParameters.offset_neg_dac = SUPPLY_100MV_NEG_VALUE - offset_Value;
		qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Negative offset DAC value: " << stParameters.offset_neg_dac;

		/*adc offset calibration*/
		try {
			value = m_m2k_powersupply->readChannel(1, false);
		} catch (libm2k::m2k_exception &e) {
			HANDLE_EXCEPTION(e)
			qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Can't read value: " << e.what();
		}
		stParameters.offset_neg_adc = offset_Value - value;
		qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Negative offset ADC value: " << stParameters.offset_neg_adc;

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
			qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Negative gain DAC value: " << stParameters.gain_neg_dac;

			/*adc gain calibration*/
			try {
				value = m_m2k_powersupply->readChannel(1, false);
			} catch (libm2k::m2k_exception &e) {
				HANDLE_EXCEPTION(e)
				qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Can't read value: " << e.what();
			}
			stParameters.gain_neg_adc =  offset_Value / (value +
						     stParameters.offset_neg_adc);
			qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Negative gain ADC value: " << stParameters.gain_neg_adc;
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
	try {
		m_m2k_powersupply->enableChannel(1, enabled);
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e)
		qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Can't enable channel: " << e.what();
	}
}

void ManualCalibration::setNegativeValue(double value)
{
	try {
		m_m2k_powersupply->pushChannel(1, value, false);
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e)
		qDebug(CAT_M2K_CALIBRATION_MANUAL) << "Can't write value: " << e.what();
	}
}

void ManualCalibration::nextButton_clicked()
{
	startCalibration();
}

void ManualCalibration::setCalibration(Calibration *cal)
{
	calib = cal;
}

void ManualCalibration::allowManualCalibScript(bool calib_en, bool calib_pref_en)
{
	if (calib_pref_en && calib_en) {
		eng->globalObject().setProperty("manual_calib",
						     eng->newQObject(api));
	} else {
		if (eng->globalObject().hasProperty("manual_calib")) {
			eng->globalObject().deleteProperty("manual_calib");
		}
	}
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
				m_m2k_context->getAdcCalibrationOffset(0))));

	startParamTable->setItem(1, 0, new QTableWidgetItem("ADC offset Ch1"));
	startParamTable->setItem(1, 1, new QTableWidgetItem(QString::number(
				m_m2k_context->getAdcCalibrationOffset(1))));

	startParamTable->setItem(2, 0, new QTableWidgetItem("ADC gain Ch0"));
	startParamTable->setItem(2, 1, new QTableWidgetItem(QString::number(
				m_m2k_context->getAdcCalibrationGain(0))));

	startParamTable->setItem(3, 0, new QTableWidgetItem("ADC gain Ch1"));
	startParamTable->setItem(3, 1, new QTableWidgetItem(QString::number(
				m_m2k_context->getAdcCalibrationGain(1))));

	startParamTable->setItem(4, 0, new QTableWidgetItem("DAC A offset"));
	startParamTable->setItem(4, 1, new QTableWidgetItem(QString::number(
				m_m2k_context->getDacCalibrationOffset(0))));

	startParamTable->setItem(5, 0, new QTableWidgetItem("DAC B offset"));
	startParamTable->setItem(5, 1, new QTableWidgetItem(QString::number(
				m_m2k_context->getDacCalibrationOffset(1))));

	startParamTable->setItem(6, 0, new QTableWidgetItem("DAC A vlsb"));
	startParamTable->setItem(6, 1, new QTableWidgetItem(QString::number(
				m_m2k_context->getDacCalibrationGain(0))));

	startParamTable->setItem(7, 0, new QTableWidgetItem("DAC B vlsb"));
	startParamTable->setItem(7, 1, new QTableWidgetItem(QString::number(
				m_m2k_context->getDacCalibrationGain(1))));

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
	QString fileName;
	QString selectedFilter;

	if (calibrationFilePath == "") {
		fileName = QFileDialog::getSaveFileName(this,
		    tr("Save file"), "", tr("Ini files (*.ini)"),
		    &selectedFilter, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
	} else {
		fileName = calibrationFilePath;
	}

	QFile file(fileName);
	QString temp_ad9963, temp_fpga;
	if (m_dmm_ad9963) {
		temp_ad9963 = QString::number(m_dmm_ad9963->readChannel("temp0").value);
	}
	if (m_dmm_xadc) {
		temp_fpga = QString::number(m_dmm_xadc->readChannel("temp0").value);
	}

	if (file.open(QIODevice::WriteOnly)) {
		QTextStream stream(&file);

		stream << "#Calibration time: " << QDate::currentDate().toString() << ", "
		       << QTime::currentTime().toString()
		       << "\n#ad9963 temperature: " << temp_ad9963
		       << tr(" °C") << "\n#FPGA temperature: "<< temp_fpga
		       << tr(" °C") << Qt::endl;

		for (int i = 0; i < paramTable->rowCount(); i++)
			stream << "cal," << paramTable->item(i,0)->text() << "="
			       << paramTable->item(i,1)->text() << Qt::endl;
	}

	file.close();

	if (calibrationFilePath != "") {
		calibrationFilePath = "";
	}
}

void ManualCalibration::restartButton_clicked()
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

void ManualCalibration::finishButton_clicked()
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
		calib->calibrateAll();
	}
	displayStartUpCalibrationValues();
}

void ManualCalibration::setCalibrationFilePath(QString path)
{
	calibrationFilePath = path;
}
