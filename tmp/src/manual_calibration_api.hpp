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

#ifndef MANUAL_CALIBRATION_API_HPP
#define MANUAL_CALIBRATION_API_HPP

#include <QObject>
#include <QProcess>
#include "manualcalibration.h"

class QJSEngine;

namespace adiscope {

class ApiObject;
class ManualCalibration;
class ManualCalibration_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(double offset_pos_dac READ getOffsetPosDac)
	Q_PROPERTY(double offset_neg_dac READ getOffsetNegDac)
	Q_PROPERTY(double gain_pos_dac READ getGainPosDac)
	Q_PROPERTY(double gain_neg_dac READ getGainNegDac)

	Q_PROPERTY(double offset_pos_adc READ getOffsetPosAdc)
	Q_PROPERTY(double offset_neg_adc READ getOffsetNegAdc)
	Q_PROPERTY(double gain_pos_adc READ getGainPosAdc)
	Q_PROPERTY(double gain_neg_adc READ getGainNegAdc)

public:
	explicit ManualCalibration_API(ManualCalibration *m_calib);
	~ManualCalibration_API() {}

	Q_INVOKABLE void autoCalibration();


	/* Starts a calib story.
	 * Returns the number of steps in this story.
	 * Returns -1 if there are no steps
	 */
	Q_INVOKABLE int start(int story);


	/* Moves the execution from the current step, to the next one.
	 * Returns the next step index for this story.
	 * Returns -1 if this was the last step.
	 */
	Q_INVOKABLE int next();


	/* Saves the params set for this story.
	 * Should be run after the last step in the story.
	 * Returns the next story index.
	 * Returns -1 if this is the last story.
	 */
	Q_INVOKABLE int finish();


	/* Save a value for the current step, in the current story.
	 */
	Q_INVOKABLE bool setParam(double value);


	/* Write all the params in the .ini calibration file.
	 */
	Q_INVOKABLE void saveCalibration(QString path);

	/* Reset to default parameters */
	Q_INVOKABLE void loadCalibration();

private:
	double getOffsetPosDac() const;
	double getOffsetNegDac() const;
	double getGainPosDac() const;
	double getGainNegDac() const;

	double getOffsetPosAdc() const;
	double getOffsetNegAdc() const;
	double getGainPosAdc() const;
	double getGainNegAdc() const;

	ManualCalibration *calib;
	int step_in_progress;
};

}

#endif /* MANUAL_CALIBRATION_API_HPP */

