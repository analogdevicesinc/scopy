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

#ifndef INSIDESCALEDRAW_H
#define INSIDESCALEDRAW_H

#include "basicscaledraw.h"
#include "scopy-gui_export.h"

namespace scopy {
/*
 * InsideScaleDraw keeps every label within the canvas when a scale is drawn inside it.
 *
 * The end major ticks sit exactly on the canvas edge, so a label centred on one would be
 * drawn half outside and clipped. This shifts the overhanging labels back in by however
 * much they overhang, which is what makes the interval bounds readable.
 *
 * It derives from BasicScaleDraw so the number formatting stays identical to the scales
 * drawn outside the canvas: prefix formatter, units, and the overlap-elimination pass all
 * come along unchanged.
 */
class SCOPY_GUI_EXPORT InsideScaleDraw : public BasicScaleDraw
{
public:
	InsideScaleDraw(PrefixFormatter *formatter, const QString &unit);

	// Shrinks the window labels are kept inside, in pixels, measured from each end of the scale.
	void setEndMargins(double start, double end);
	double startMargin() const;
	double endMargin() const;

protected:
	void draw(QPainter *painter, const QPalette &palette) const override;
	void drawLabel(QPainter *painter, double value) const override;

private:
	bool labelShift(const QFont &font, double value, QRect &bounds, double &shift) const;

	double m_startMargin;
	double m_endMargin;
};
} // namespace scopy

#endif // INSIDESCALEDRAW_H
