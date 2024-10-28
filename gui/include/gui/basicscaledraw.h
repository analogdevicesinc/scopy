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

#ifndef BASICSCALEDRAW_H
#define BASICSCALEDRAW_H

#include "scopy-gui_export.h"
#include <QColor>
#include <QwtScaleDraw>
#include "plot_utils.hpp"

namespace scopy {
/*
 * BasicScaleDraw class overrides the way the major values are being displayed.
 */
class SCOPY_GUI_EXPORT BasicScaleDraw : public QwtScaleDraw
{
public:
	BasicScaleDraw(const QString &unit_type = "");
	BasicScaleDraw(PrefixFormatter *, const QString &);
	QwtText label(double) const;

	void setFloatPrecision(unsigned int numDigits);
	unsigned int getFloatPrecison() const;

	void setUnitType(const QString &unit);
	QString getUnitType() const;

	void setColor(QColor color);

	void setDisplayScale(double value);
	void setFormatter(PrefixFormatter *formatter);

	void enableDeltaLabel(bool enable);
	void setUnitsEnabled(bool enable);

protected:
	virtual void draw(QPainter *, const QPalette &) const;

private:
	int m_floatPrecision;
	QString m_unit;
	PrefixFormatter *m_formatter;
	QColor m_color;
	double m_displayScale;
	mutable unsigned int m_nrTicks;
	mutable bool m_shouldDrawMiddleDelta;
	bool m_delta;
	bool m_unitsEn;
};
} // namespace scopy

#endif // BASICSCALEDRAW_H
