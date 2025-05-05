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

#ifndef TIMEPLOTCOMPONENTCHANNEL_H
#define TIMEPLOTCOMPONENTCHANNEL_H

#include "scopy-adc_export.h"
#include <QObject>
#include <gui/toolcomponent.h>
#include <gui/channelcomponent.h>
#include <timeplotcomponent.h>
#include <gui/plotcomponent.h>

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT TimePlotComponentChannel : public QObject, public PlotComponentChannel
{
	Q_OBJECT
public:
	TimePlotComponentChannel(ChannelComponent *ch, TimePlotComponent *plotComponent, QObject *parent);
	~TimePlotComponentChannel();

	QWidget *createCurveMenu(QWidget *parent);
	ChannelComponent *channelComponent() override;
	PlotComponent *plotComponent() override;
	PlotChannel *plotChannel() override;

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onNewData(const float *xData_, const float *yData_, size_t size, bool copy) override;
	void setXyXData(const float *);
	void lockYAxis(bool);
	void refreshData(bool copy);

	void initPlotComponent(PlotComponent *plotComponent) override;
	void deinitPlotComponent() override;

public:
	PlotChannel *m_timePlotCh = nullptr;
	PlotAxis *m_timePlotYAxis = nullptr;
	PlotAxisHandle *m_timePlotAxisHandle = nullptr;

	PlotChannel *m_xyPlotCh = nullptr;
	PlotAxis *m_xyPlotYAxis = nullptr;

	TimePlotComponent *m_plotComponent = nullptr;
	ChannelComponent *m_ch;
	const float *m_xyXData = 0;
	bool m_singleYMode = false;
	bool m_enabled;
};
} // namespace adc
} // namespace scopy
#endif // TIMEPLOTCOMPONENTCHANNEL_H
