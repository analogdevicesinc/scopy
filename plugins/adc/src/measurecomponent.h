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

#ifndef MEASURECOMPONENT_H
#define MEASURECOMPONENT_H

#include "scopy-adc_export.h"
#include "gui/widgets/measurementsettings.h"
#include "gui/widgets/menucontrolbutton.h"
#include <gui/interfaces.h>
#include <gui/toolcomponent.h>
#include "adcinterfaces.h"

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT MeasureComponent : public QObject, public ToolComponent
{
public:
	MeasureComponent(ToolTemplate *tool, QButtonGroup *btngroup, MeasurementPanelInterface *p, QObject *parent);
	MeasurementSettings *measureSettings();

private:
	void setupMeasureButtonHelper(MenuControlButton *);
	MeasurementSettings *m_measureSettings;
	QButtonGroup *hoverBtnGroup;

	QString measureMenuId = "measure";
	QString statsMenuId = "stats";
	MenuControlButton *measure;
	MeasurementPanelInterface *m_measurementPanelInterface;
};
} // namespace adc
} // namespace scopy

#endif // MEASURECOMPONENT_H
