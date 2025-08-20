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

#ifndef IPLOT_H
#define IPLOT_H

#include "qiqutils.h"

#include <QWidget>
#include <qiqcontroller/qiqplotinfo.h>
namespace scopy::qiqplugin {

class IPlot : public QObject
{
	Q_OBJECT
public:
	virtual int id() = 0;
	virtual void init(QIQPlotInfo info, int samplingFreq) = 0;
	virtual void updateData(QList<CurveData> curveData) = 0;
	virtual void setSamplingFreq(int samplingFreq) = 0;
	virtual QWidget *widget() = 0;
	virtual ~IPlot() = default;
};

} // namespace scopy::qiqplugin

#endif // IPLOT_H
