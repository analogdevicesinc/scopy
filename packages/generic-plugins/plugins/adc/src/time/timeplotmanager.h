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

#ifndef TIMEPLOTMANAGER_H
#define TIMEPLOTMANAGER_H
#include "plotbufferpreviewer.h"
#include "scopy-adc_export.h"
#include <QWidget>
#include "adcinterfaces.h"
#include <measurementpanel.h>
#include "time/timeplotcomponent.h"
#include <gui/channelcomponent.h>
#include <gui/plotmanager.h>

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT TimePlotManager : public PlotManager
{
	Q_OBJECT
public:
	TimePlotManager(QString name = "TimePlotManager", QWidget *parent = nullptr);
	~TimePlotManager();

	virtual uint32_t addPlot(QString name) override;
	virtual void removePlot(uint32_t uuid) override;
	TimePlotComponent *plot(uint32_t uuid);

private:
	PlotComponent *m_primary;
	PlotBufferPreviewer *m_plotpreviewer;
	AnalogBufferPreviewer *m_bufferpreviewer;
	void multiPlotUpdate();

	// void syncCursors();
	void syncNavigatorAndCursors(PlotComponent *);
	void syncAllPlotNavigatorsAndCursors();
};
} // namespace adc
} // namespace scopy
#endif // TIMEPLOTMANAGER_H
