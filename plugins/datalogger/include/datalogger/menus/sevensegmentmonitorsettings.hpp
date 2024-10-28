/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef SEVENSEGMENTMONITORSETTINGS_HPP
#define SEVENSEGMENTMONITORSETTINGS_HPP

#include <QLineEdit>
#include <QWidget>
#include <menuonoffswitch.h>
#include "scopy-datalogger_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGER_EXPORT SevenSegmentMonitorSettings : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit SevenSegmentMonitorSettings(QWidget *parent = nullptr);

Q_SIGNALS:
	void peakHolderToggled(bool toggled);
	void precisionChanged(int precision);
signals:

private:
	MenuOnOffSwitch *peakHolderToggle;
	QLineEdit *precision;
	void changePrecision();
};
} // namespace datamonitor
} // namespace scopy
#endif // SEVENSEGMENTMONITORSETTINGS_HPP
