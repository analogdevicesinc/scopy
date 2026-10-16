/*
 * Copyright (c) 2025 Analog Devices Inc.
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
 *
 */

#ifndef CN0511_H
#define CN0511_H

#include "scopy-cn0511_export.h"
#include <QBoxLayout>
#include <QWidget>
#include <animatedloadingbutton.h>
#include <menuspinbox.h>
#include <tooltemplate.h>
#include <animatedrefreshbtn.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <QCoro/QCoroTask>
#include <QFuture>

namespace scopy {
class IIOWidgetGroup;

namespace component {
class Channel;
class Context;
class Device;
} // namespace component

namespace cn0511 {

class SCOPY_CN0511_EXPORT CN0511 : public QWidget
{
	Q_OBJECT
public:
	CN0511(component::Context *ctx, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~CN0511();

private Q_SLOTS:
	void applyCalibration();
	void refresh();

private:
	friend class Cn0511_API;
	component::Context *m_ctx = nullptr;
	IIOWidgetGroup *m_group = nullptr;
	ToolTemplate *m_tool = nullptr;
	AnimatedRefreshBtn *m_refreshButton = nullptr;

	component::Device *m_dac = nullptr;
	component::Channel *m_dacCh = nullptr;
	IIOWidget *m_dacAmpEnWidget = nullptr;

	gui::MenuSpinbox *m_freqSpinBox = nullptr;
	gui::MenuSpinbox *m_ampSpinBox = nullptr;
	AnimatedLoadingButton *m_enterBtn = nullptr;

	double m_calibFreq = 4500000000.0;
	int m_amplitude = 0;

	QCoro::Task<void> m_refreshTask;
	QFuture<void> m_calibFuture;

	QWidget *generateSingleToneSection(QWidget *parent);
	QWidget *generateDacAmpSection(QWidget *parent);
	QCoro::Task<void> readFrequencyFromDevice();
};
} // namespace cn0511
} // namespace scopy
#endif // CN0511_H
