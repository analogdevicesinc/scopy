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

#ifndef SEVENSEGMENTDISPLAY_H
#define SEVENSEGMENTDISPLAY_H

#include "datamonitormodel.hpp"
#include "scopy-datalogger_export.h"
#include "sevensegmentmonitor.hpp"

#include <QBoxLayout>
#include <QMap>
#include <QWidget>

namespace scopy::datamonitor {
class SCOPY_DATALOGGER_EXPORT SevenSegmentDisplay : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit SevenSegmentDisplay(QWidget *parent = nullptr);
	~SevenSegmentDisplay();

	void generateSegment(DataMonitorModel *model);
	void removeSegment(QString segment);

	void updatePrecision(int precision);
	void togglePeakHolder(bool toggle);
signals:

private:
	QVBoxLayout *layout;
	QMap<QString, SevenSegmentMonitor *> *monitorList;
};
} // namespace scopy::datamonitor
#endif // SEVENSEGMENTDISPLAY_H
