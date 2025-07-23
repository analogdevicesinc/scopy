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

#ifndef FFTPLOTMANAGER_H
#define FFTPLOTMANAGER_H
#include "scopy-adc_export.h"
#include <QWidget>
#include <gui/interfaces.h>
#include <gui/channelcomponent.h>
#include <measurementpanel.h>
#include <gui/plotmanager.h>
#include <freq/fftplotcomponent.h>

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT FFTPlotManager : public PlotManager
{
	Q_OBJECT
public:
	FFTPlotManager(QString name = "FFTPlotManager", QWidget *parent = nullptr);
	~FFTPlotManager();

	virtual uint32_t addPlot(QString name) override;
	virtual void removePlot(uint32_t uuid) override;
	FFTPlotComponent *plot(uint32_t uuid);
	void enableMeasurementPanel(bool) override;

private:
	PlotComponent *m_primary;
	void multiPlotUpdate();

	void syncNavigatorAndCursors(PlotComponent *);
	void syncAllPlotNavigatorsAndCursors();
};
} // namespace adc
} // namespace scopy
#endif // FFTPLOTMANAGER_H
