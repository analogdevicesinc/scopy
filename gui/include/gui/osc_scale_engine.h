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

#ifndef M2K_OSC_SCALE_ENGINE_H
#define M2K_OSC_SCALE_ENGINE_H

#include <qwt_scale_engine.h>
#include "scopygui_export.h"

class SCOPYGUI_EXPORT OscScaleEngine: public QwtLinearScaleEngine
{
public:
	OscScaleEngine();
	virtual ~OscScaleEngine();

	void setMajorTicksCount(uint majorTicks);
	uint majorTicksCount();

	void setMinorTicksCount(uint minorTicks);
	uint minorTicksCount();
    void showZero(bool en);
	virtual void autoScale( int maxSteps,
		double &x1, double &x2, double &stepSize ) const;

	virtual QwtScaleDiv divideScale(double x1, double x2, int maxMajorSteps,
					int maxMinorSteps, double stepSize = 0.0) const;

protected:
	void buildTicks(
		const QwtInterval &, double stepSize, int maxMinSteps,
		QList<double> ticks[QwtScaleDiv::NTickTypes] ) const;

	void buildMinorTicks( const QList<double>& majorTicks,
		int maxMinorSteps, double stepSize,
		QList<double> &minorTicks, QList<double> &mediumTicks ) const;

    QList<double> buildMajorTicks(const QwtInterval &interval, double stepSize) const;
private:
	uint m_majorTicks; // number of major ticks a scale should have
	uint m_minorTicks; // number of minor ticks between two major ticks
    bool m_showZero;
};

#endif /* M2K_OSC_SCALE_ENGINE_H */
