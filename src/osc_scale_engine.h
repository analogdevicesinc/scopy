/*
 * Copyright 2016 Analog Devices, Inc.
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

#ifndef M2K_OSC_SCALE_ENGINE_H
#define M2K_OSC_SCALE_ENGINE_H

#include <qwt_scale_engine.h>

class OscScaleEngine: public QwtLinearScaleEngine
{
public:
	OscScaleEngine();
	virtual ~OscScaleEngine();

	void setMajorTicksCount(uint majorTicks);
	uint majorTicksCount();

	void setMinorTicksCount(uint minorTicks);
	uint minorTicksCount();

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
private:
	uint m_majorTicks; // number of major ticks a scale should have
	uint m_minorTicks; // number of minor ticks between two major ticks
};

#endif /* M2K_OSC_SCALE_ENGINE_H */
