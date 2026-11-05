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

#ifndef VALUEMONITORWIDGET_H
#define VALUEMONITORWIDGET_H

#include "scopy-gui_export.h"

#include <QColor>
#include <QFrame>
#include <QLabel>
#include <QString>

namespace scopy {
class LcdNumber;

class SCOPY_GUI_EXPORT ValueMonitorWidget : public QFrame
{
	Q_OBJECT
public:
	ValueMonitorWidget(const QString &name, const QString &unit, const QString &rowLabel = QString(),
			   unsigned precision = 3, const QColor &color = QColor(), bool peakHoldVisible = true,
			   QWidget *parent = nullptr);
	explicit ValueMonitorWidget(QWidget *parent = nullptr);
	~ValueMonitorWidget() override = default;

	void setMonitorName(const QString &name);
	void setUnit(const QString &unit);
	void setRowLabel(const QString &label);
	void setPrecision(unsigned precision);
	void setColor(const QColor &color);

public Q_SLOTS:
	void setValue(double value);
	void setMin(double value);
	void setMax(double value);
	void setPeakHoldVisible(bool visible);

private:
	void applyColor();

	QColor m_color;
	QLabel *m_name = nullptr;
	QLabel *m_unit = nullptr;
	QLabel *m_rowLabel = nullptr;
	LcdNumber *m_value = nullptr;
	LcdNumber *m_min = nullptr;
	LcdNumber *m_max = nullptr;
	QLabel *m_minLabel = nullptr;
	QLabel *m_maxLabel = nullptr;
	QWidget *m_peakHold = nullptr;
};
} // namespace scopy
#endif // VALUEMONITORWIDGET_H
