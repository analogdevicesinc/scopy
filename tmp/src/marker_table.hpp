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
#ifndef MARKER_TABLE_HPP
#define MARKER_TABLE_HPP

#include <QWidget>

class QStandardItemModel;

namespace Ui {
class MarkerTable;
}

namespace adiscope {

class MarkerTable : public QWidget
{
	Q_OBJECT

public:
	explicit MarkerTable(QWidget *parent = 0);
	~MarkerTable();

	void addMarker(int mkIdx, int chIdx, const QString& name,
		double frequency, double magnitude, const QString& type);
	void removeMarker(int mkIdx, int chIdx);
	void updateMarker(int mkIdx, int chIdx, double frequency,
		double magnitude, const QString& type);
	bool isMarker(int mkIdx, int chIdx);

private:
	enum Columns {
		COL_ID = 0,
		COL_NAME = 1,
		COL_CH = 2,
		COL_FREQ = 3,
		COL_MAGN = 4,
		COL_TYPE = 5,
		NUM_COLUMNS
	};

	Ui::MarkerTable *ui;
	QStandardItemModel *model;

	int rowOfMarker(int mkIdx, int chIdx) const;
};

} /* namespace adiscope */

#endif // MARKER_TABLE_HPP
