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

#ifndef MEASUREMENTSELECTOR_H
#define MEASUREMENTSELECTOR_H

#include "scopy-gui_export.h"

#include <QCheckBox>
#include <QLabel>
#include <QMap>
#include <QVBoxLayout>
#include <QWidget>

namespace scopy {

class SCOPY_GUI_EXPORT MeasurementSelectorItem : public QWidget
{
	Q_OBJECT
	friend class StyleHelper;

public:
	MeasurementSelectorItem(QString name, QString icon, QWidget *parent);
	~MeasurementSelectorItem();

	QCheckBox *measureCheckbox() const;
	QCheckBox *statsCheckbox() const;

private:
	QLabel *m_icon;
	QLabel *m_name;
	QCheckBox *m_measureCheckbox;
	QCheckBox *m_statsCheckbox;
};

class SCOPY_GUI_EXPORT MeasurementSelector : public QWidget
{
	Q_OBJECT
public:
	MeasurementSelector(QWidget *parent = nullptr);
	~MeasurementSelector();

	void addMeasurement(QString name, QString icon);
	void removeMeasurement(QString name);
	MeasurementSelectorItem *measurement(QString name);

	void toggleAllMeasurement(bool b);
	void toggleAllStats(bool b);

private:
	QMap<QString, MeasurementSelectorItem *> m_map;
	QVBoxLayout *lay;
};
} // namespace scopy

#endif // MEASUREMENTSELECTOR_H
