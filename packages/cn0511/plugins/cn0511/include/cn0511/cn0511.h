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
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <tooltemplate.h>
#include <animatedrefreshbtn.h>
#include <iio-widgets/iiowidgetbuilder.h>

struct iio_context;
struct iio_device;
struct iio_channel;

namespace scopy {
class IIOWidgetGroup;
namespace cn0511 {

class SCOPY_CN0511_EXPORT CN0511 : public QWidget
{
	Q_OBJECT
public:
	CN0511(iio_context *ctx, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~CN0511();

Q_SIGNALS:
	void readRequested();

private Q_SLOTS:
	void applyCalibration();

private:
	iio_context *m_ctx = nullptr;
	IIOWidgetGroup *m_group = nullptr;
	ToolTemplate *m_tool = nullptr;
	AnimatedRefreshBtn *m_refreshButton = nullptr;

	iio_device *m_dac = nullptr;
	iio_channel *m_dacCh = nullptr;

	QDoubleSpinBox *m_freqSpinBox = nullptr;
	QSpinBox *m_ampSpinBox = nullptr;

	double m_calibFreq = 4500000000.0;
	int m_amplitude = 0;

	QWidget *generateSingleToneSection(QWidget *parent);
	QWidget *generateDacAmpSection(QWidget *parent);
};
} // namespace cn0511
} // namespace scopy
#endif // CN0511_H
