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
#ifndef LIMITEDPLOTZOOMER_H
#define LIMITEDPLOTZOOMER_H

#include <qwt_plot_zoomer.h>

namespace scopy {
namespace gui {

class LimitedPlotZoomer : public QwtPlotZoomer
{
public:
	LimitedPlotZoomer(QWidget*, bool doReplot = false);
	void resetZoom();

	void setBoundVertical(bool bound);

protected:
	virtual void zoom(const QRectF&);
	virtual QSizeF minZoomSize() const override;

private:
	bool m_boundVertical;
};
} // namespace gui
} // namespace scopy

#endif // LIMITEDPLOTZOOMER_H
