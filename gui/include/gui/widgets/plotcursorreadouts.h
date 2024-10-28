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

#ifndef PLOTCURSORREADOUTS_H
#define PLOTCURSORREADOUTS_H

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include <plot_utils.hpp>
#include <scopy-gui_export.h>
#include <stylehelper.h>

namespace scopy {
class SCOPY_GUI_EXPORT PlotCursorReadouts : public QWidget
{
	Q_OBJECT
public:
	PlotCursorReadouts(QWidget *parent = nullptr);
	~PlotCursorReadouts();

	bool isXVisible();
	bool isYVisible();

public Q_SLOTS:
	void setY1(double val);
	void setY2(double val);
	void setX1(double val);
	void setX2(double val);
	void setXVisible(bool visible);
	void setYVisible(bool visible);
	void update();

	void setYUnits(QString unit);
	void setXUnits(QString unit);

	void setXFormatter(PrefixFormatter *formatter);
	void setYFormatter(PrefixFormatter *formatter);

private:
	QWidget *y_contents;
	QWidget *x_contents;
	double y1, y2, x1, x2;
	QLabel *y1_val, *y2_val, *x1_val, *x2_val;
	QLabel *deltaY_val, *deltaX_val;
	QLabel *invDeltaX_val;
	PrefixFormatter *xFormatter, *yFormatter;
	QString xUnit, yUnit;

	void initContent();
};
} // namespace scopy

#endif // PLOTCURSORREADOUTS_H
