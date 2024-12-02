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

#ifndef OSC_SCALE_ZOOMER_H
#define OSC_SCALE_ZOOMER_H

#include <qwt_plot_zoomer.h>
#include "limitedplotzoomer.h"

namespace adiscope {
	class OscScaleZoomer : public LimitedPlotZoomer
	{
		Q_OBJECT

		Q_PROPERTY(QColor color READ getColor WRITE setColor);

	public:
		explicit OscScaleZoomer(QWidget *parent);
		~OscScaleZoomer();

		void cancel();

		QColor getColor() const;
		void setColor(const QColor& color);

	private:
		QwtText trackerText(const QPoint& pos) const;
	};
}

#endif /* OSC_SCALE_ZOOMER_H */
