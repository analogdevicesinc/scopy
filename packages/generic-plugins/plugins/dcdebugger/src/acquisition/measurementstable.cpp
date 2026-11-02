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
 *
 */

#include "measurementstable.h"

#include <QHeaderView>
#include <QPixmap>

#include <cmath>

#include <style.h>

using namespace scopy;
using namespace scopy::dcdebugger;

QString scopy::dcdebugger::siFormat(double v, const QString &unit, int digits)
{
	if(!std::isfinite(v)) {
		return QStringLiteral("—");
	}
	static const struct
	{
		double scale;
		const char *prefix;
	} prefixes[] = {{1e9, "G"}, {1e6, "M"}, {1e3, "k"}, {1, ""}, {1e-3, "m"}, {1e-6, "µ"}, {1e-9, "n"}};
	const double a = std::abs(v);
	for(const auto &p : prefixes) {
		if(a >= p.scale || p.scale == 1e-9) {
			if(a == 0) {
				break;
			}
			return QString::number(v / p.scale, 'g', digits) + " " + QString::fromUtf8(p.prefix) + unit;
		}
	}
	return QStringLiteral("0 ") + unit;
}

MeasurementsTable::MeasurementsTable(QWidget *parent)
	: QTableWidget(parent)
{
	const QStringList headers = {"Channel", "Min", "Max", "Mean", "RMS", "AC RMS", "Pk-Pk", "Freq", "Peak"};
	setColumnCount(headers.size());
	setHorizontalHeaderLabels(headers);
	horizontalHeader()->setStretchLastSection(true);
	horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	verticalHeader()->setVisible(false);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionMode(QAbstractItemView::NoSelection);
	setFocusPolicy(Qt::NoFocus);
	Style::setStyle(this, style::properties::widget::tableViewWidget);
}

void MeasurementsTable::setRows(const QList<MeasurementRow> &rows, const QString &unit)
{
	setRowCount(rows.size());
	for(int r = 0; r < rows.size(); ++r) {
		const MeasurementRow &row = rows[r];
		const Measurements &m = row.values;
		QPixmap swatch(10, 10);
		swatch.fill(row.color);
		auto *name = new QTableWidgetItem(QIcon(swatch), row.channel);
		setItem(r, 0, name);
		const QStringList cells = {siFormat(m.min, unit),
					   siFormat(m.max, unit),
					   siFormat(m.mean, unit),
					   siFormat(m.rms, unit),
					   siFormat(m.acRms, unit),
					   siFormat(m.pkpk, unit),
					   m.frequency < 0 ? QStringLiteral("—") : siFormat(m.frequency, "Hz"),
					   row.peak.isEmpty() ? QStringLiteral("—") : row.peak};
		for(int c = 0; c < cells.size(); ++c) {
			auto *item = new QTableWidgetItem(cells[c]);
			item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			setItem(r, c + 1, item);
		}
	}
}

#include "moc_measurementstable.cpp"
