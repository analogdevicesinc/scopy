/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ADALM2000VOLTMETER_API_H
#define ADALM2000VOLTMETER_API_H

#include "scopy-adalm2000_export.h"

#include <QList>
#include <QString>

#include <pluginbase/apiobject.h>

namespace scopy::adalm2000 {

class Adalm2000VoltmeterTool;

class SCOPY_ADALM2000_EXPORT Adalm2000Voltmeter_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(bool running READ running WRITE setRunning STORED false)
	Q_PROPERTY(bool mode_ac_ch1 READ modeAcCh1 WRITE setModeAcCh1)
	Q_PROPERTY(bool mode_ac_ch2 READ modeAcCh2 WRITE setModeAcCh2)
	Q_PROPERTY(QList<int> gainModes READ gainModes WRITE setGainModes)
	Q_PROPERTY(double value_ch1 READ valueCh1 STORED false)
	Q_PROPERTY(double value_ch2 READ valueCh2 STORED false)
	Q_PROPERTY(bool plot_visible_ch1 READ plotVisibleCh1 WRITE setPlotVisibleCh1)
	Q_PROPERTY(bool plot_visible_ch2 READ plotVisibleCh2 WRITE setPlotVisibleCh2)
	Q_PROPERTY(bool peak_hold_en READ peakHoldEn WRITE setPeakHoldEn)
	Q_PROPERTY(double peak_min_ch1 READ peakMinCh1 STORED false)
	Q_PROPERTY(double peak_min_ch2 READ peakMinCh2 STORED false)
	Q_PROPERTY(double peak_max_ch1 READ peakMaxCh1 STORED false)
	Q_PROPERTY(double peak_max_ch2 READ peakMaxCh2 STORED false)
	Q_PROPERTY(bool data_logging_en READ dataLoggingEn WRITE setDataLoggingEn)
	// Writing this disarms logging, so set it BEFORE data_logging_en, never after.
	Q_PROPERTY(QString data_logging_path READ dataLoggingPath WRITE setDataLoggingPath)

public:
	explicit Adalm2000Voltmeter_API(Adalm2000VoltmeterTool *tool);
	~Adalm2000Voltmeter_API() override;

	bool running() const;
	void setRunning(bool on);

	bool modeAcCh1() const;
	void setModeAcCh1(bool ac);

	bool modeAcCh2() const;
	void setModeAcCh2(bool ac);

	QList<int> gainModes() const;
	void setGainModes(const QList<int> &modes);

	double valueCh1() const;
	double valueCh2() const;

	bool plotVisibleCh1() const;
	void setPlotVisibleCh1(bool on);

	bool plotVisibleCh2() const;
	void setPlotVisibleCh2(bool on);

	bool peakHoldEn() const;
	void setPeakHoldEn(bool en);

	double peakMinCh1() const;
	double peakMinCh2() const;
	double peakMaxCh1() const;
	double peakMaxCh2() const;

	bool dataLoggingEn() const;
	void setDataLoggingEn(bool en);

	QString dataLoggingPath() const;
	void setDataLoggingPath(const QString &path);

	Q_INVOKABLE void resetPeakHold();

private:
	Adalm2000VoltmeterTool *m_tool;
};

} // namespace scopy::adalm2000

#endif // ADALM2000VOLTMETER_API_H
