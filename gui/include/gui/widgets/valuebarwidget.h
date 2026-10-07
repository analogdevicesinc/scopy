/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 */

#ifndef VALUEBARWIDGET_H
#define VALUEBARWIDGET_H

#include "scopy-gui_export.h"

#include <QColor>
#include <qwt_thermo.h>

namespace scopy {

/**
 * @brief A level bar for a single scalar value, with a scale alongside it.
 */
class SCOPY_GUI_EXPORT ValueBarWidget : public QwtThermo
{
	Q_OBJECT
public:
	explicit ValueBarWidget(QWidget *parent = nullptr);
	~ValueBarWidget() override = default;

	void setRange(double lower, double upper);
	void setTickCounts(int major, int minor);
	void setBarColor(const QColor &color);
	QColor barColor() const;

private:
	QColor m_barColor;
};

} // namespace scopy
#endif // VALUEBARWIDGET_H
