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

#ifndef MANUALCALIBRATION_H
#define MANUALCALIBRATION_H

#include <iio.h>

/*Qt includes*/
#include <QListWidgetItem>
#include <QObject>
#include <QString>
#include <QTableWidget>

/*Local includes*/
#include "detachedWindow.hpp"
#include "filter.hpp"
#include "tool.hpp"
#include "tool_launcher.hpp"

class QJSEngine;

namespace Ui {
class ManualCalibration;
class CalibrationTemplate;
} // namespace Ui

namespace adiscope {

struct stCalibStory {
	int calibProcedure;
	int calibStep;
	QStringList story;
	QString storyName;
};

struct stCalibParam {
	double offset_pos_dac;
	double offset_neg_dac;
	double gain_pos_dac;
	double gain_neg_dac;

	double offset_pos_adc;
	double offset_neg_adc;
	double gain_pos_adc;
	double gain_neg_adc;
};

enum calibrations {
	POSITIVE_OFFSET = 0,
	NEGATIVE_OFFSET,
	CALIB_PARAM,
	START_CALIB_PARAM
};

enum steps { STEP1 = 0, STEP2, STEP3, STEP4 };

class ManualCalibration_API;
class ManualCalibration : public Tool {
	friend class ManualCalibration_API;
	friend class ToolLauncher_API;

	Q_OBJECT

public:
	explicit ManualCalibration(struct iio_context *ctx, Filter *filt,
				   ToolMenuItem *toolMenuItem,
				   QJSEngine *engine, ToolLauncher *parent = 0,
				   Calibration *cal = 0);

	~ManualCalibration();

	void startCalibration();
	void setCalibration(Calibration *cal);
	void allowManualCalibScript(bool calib_en, bool calib_pref_en);

private:
	void setupPowerSupplyIio(void);

	void positivePowerSupplySetup();
	void positivePowerSupplyParam(const int step);
	void setEnablePositiveSuppply(bool enabled);
	void setPositiveValue(double value);

	void negativePowerSupplySetup();
	void negativePowerSupplyParam(const int step);
	void setEnableNegativeSuppply(bool enabled);
	void setNegativeValue(double value);

	void displayStartUpCalibrationValues(void);
	void initParameters(void);
	void updateParameters(void);
	void setCalibrationFilePath(QString path);

private Q_SLOTS:
	void on_calibList_itemClicked(QListWidgetItem *item);

	void on_nextButton_clicked();
	void nextStep();

	void on_loadButton_clicked();

	void on_saveButton_clicked();

	void on_restartButton_clicked();

	void on_finishButton_clicked();

	void on_autoButton_clicked();

private:
	Ui::ManualCalibration *ui;
	struct iio_channel *ch1w, *ch2w, *ch1r, *ch2r, *pd_pos, *pd_neg;
	Ui::CalibrationTemplate *TempUi;
	QWidget *TempWidget;
	QTableWidget *paramTable;
	QTableWidget *startParamTable;

	QPushButton *menuRunButton;
	Filter *filter;
	QJSEngine *eng;

	Calibration *calib;

	struct stCalibStory stCalibrationStory;
	struct stCalibParam stParameters;
	QStringList calibListString;
	QMap<QString, int> calibOption;

	QString calibrationFilePath;
};
} // namespace adiscope
#endif // MANUALCALIBRATION_H
