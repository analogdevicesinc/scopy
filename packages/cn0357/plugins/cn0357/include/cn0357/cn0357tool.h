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

#ifndef CN0357TOOL_H
#define CN0357TOOL_H

#include "scopy-cn0357_export.h"
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QWidget>
#include <animatedrefreshbtn.h>
#include <gui/widgets/menuspinbox.h>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <iio.h>
#include <tooltemplate.h>

namespace scopy::cn0357 {

class SCOPY_CN0357_EXPORT Cn0357Tool : public QWidget
{
	Q_OBJECT

	friend class Cn0357_API;

public:
	explicit Cn0357Tool(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent = nullptr);
	~Cn0357Tool() = default;

Q_SIGNALS:
	void readRequested();

private Q_SLOTS:
	void onFeedbackTypeChanged(int index);
	void onRdacSpinChanged(double raw);
	void onFixedResistorChanged(double value);
	void onSensorSensitivityChanged(double value);
	void onAdcMvChanged(QString value);
	void updateCoefficients();

private:
	void setupUi();
	void findDevicesAndChannels();
	QWidget *createAdcSettingsSection(QWidget *parent);
	QWidget *createFeedbackSettingsSection(QWidget *parent);
	QWidget *createSystemSection(QWidget *parent);

	// IIO handles
	iio_context *m_ctx;
	IIOWidgetGroup *m_group;
	iio_device *m_adcDev = nullptr;
	iio_channel *m_adcCh = nullptr;	 // voltage0 (in)
	iio_channel *m_pwrCh = nullptr;	 // supply (in)
	iio_channel *m_rdacCh = nullptr; // dpot voltage0 (out)

	// UI structure
	ToolTemplate *m_tool;
	AnimatedRefreshBtn *m_refreshButton;
	QTimer *m_updateTimer;

	// Feedback Settings widgets
	QComboBox *m_feedbackType = nullptr;
	gui::MenuSpinbox *m_rdacSpinBox = nullptr; // RDAC raw value (0-1023, Rheostat mode)
	QPushButton *m_programRdacBtn = nullptr;
	gui::MenuSpinbox *m_fixedRes = nullptr;
	QLabel *m_fixedResLabel = nullptr;

	// System / Measurements widgets
	QLineEdit *m_ppmDisplay = nullptr;
	IIOWidget *m_adcMvWidget = nullptr;   // ReadOnly, voltage0-voltage0 raw → mV
	IIOWidget *m_supplyVWidget = nullptr; // ReadOnly, supply raw → V

	// System / Data widgets
	gui::MenuSpinbox *m_sensorSensitivity = nullptr;
	QLineEdit *m_feedbackResDisplay = nullptr;
	QLineEdit *m_ppmMvDisplay = nullptr;
	QLineEdit *m_mvPpmDisplay = nullptr;

	// Computed state
	double m_feedbackResistance = 0.0;
	double m_sensorSens = 65.0;
	double m_adcMv = 0.0;
};

} // namespace scopy::cn0357
#endif // CN0357TOOL_H
