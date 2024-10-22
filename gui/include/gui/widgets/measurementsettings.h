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

#ifndef MEASUREMENTSETTINGS_H
#define MEASUREMENTSETTINGS_H

#include "menusectionwidget.h"
#include "scopy-gui_export.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <menuonoffswitch.h>

namespace scopy {
class SCOPY_GUI_EXPORT MeasurementSettings : public QWidget
{
	Q_OBJECT
public:
	typedef enum
	{
		MPM_SORT_CHANNEL,
		MPM_SORT_TYPE
	} MeasurementSortingType;
	MeasurementSettings(QWidget *parent = nullptr);
	~MeasurementSettings();

	bool measurementEnabled();
	bool statsEnabled();
	bool markerEnabled();

	MenuSectionWidget *getMarkerSection() const;

	MenuSectionWidget *getStatsSection() const;

	MenuSectionWidget *getMeasureSection() const;

Q_SIGNALS:
	void toggleAllMeasurements(bool);
	void toggleAllStats(bool);
	void sortMeasurements(MeasurementSortingType type);
	void sortStats(MeasurementSortingType type);
	void enableMeasurementPanel(bool b);
	void enableStatsPanel(bool b);
	void enableMarkerPanel(bool b);

private:
	MenuOnOffSwitch *measurePanelSwitch;
	MenuOnOffSwitch *statsPanelSwitch;
	MenuOnOffSwitch *markerPanelSwitch;

	MenuSectionWidget *markerSection;
	MenuSectionWidget *statsSection;
	MenuSectionWidget *measureSection;
};
} // namespace scopy

#endif // MEASUREMENTSETTINGS_H
