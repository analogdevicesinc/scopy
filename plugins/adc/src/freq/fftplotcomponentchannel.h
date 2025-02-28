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

#ifndef FFTPLOTCOMPONENTCHANNEL_H
#define FFTPLOTCOMPONENTCHANNEL_H

#include "scopy-adc_export.h"
#include <QObject>
#include "fftplotcomponent.h"
#include <toolcomponent.h>
#include <channelcomponent.h>
#include <plotcomponent.h>
#include <fftmarkercontroller.hpp>

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT FFTPlotComponentChannel : public QObject, public PlotComponentChannel
{
	Q_OBJECT
public:
	FFTPlotComponentChannel(ChannelComponent *ch, FFTPlotComponent *plotComponent, QObject *parent);
	~FFTPlotComponentChannel();

	QWidget *createCurveMenu(QWidget *parent);
	ChannelComponent *channelComponent() override;
	PlotComponent *plotComponent() override;
	PlotChannel *plotChannel() override;
	FFTMarkerController *markerController() const;

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onNewData(const float *xData_, const float *yData_, size_t size, bool copy) override;
	void lockYAxis(bool);
	void refreshData(bool copy);

	void initPlotComponent(PlotComponent *plotComponent) override;
	void deinitPlotComponent() override;

public:
	PlotChannel *m_fftPlotCh = nullptr;
	PlotAxis *m_fftPlotYAxis = nullptr;
	PlotAxisHandle *m_fftPlotAxisHandle = nullptr;

	FFTPlotComponent *m_plotComponent = nullptr;
	ChannelComponent *m_ch;
	bool m_singleYMode = false;
	bool m_enabled;

private:
	FFTMarkerController *m_markerController;
};
} // namespace adc
} // namespace scopy
#endif // FFTPLOTCOMPONENTCHANNEL_H
