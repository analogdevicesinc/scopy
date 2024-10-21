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

#ifndef IMPORTCHANNELCOMPONENT_H
#define IMPORTCHANNELCOMPONENT_H

#include <gui/channelcomponent.h>
#include <gui/plotautoscaler.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>

#include <adcacquisitionmanager.h>
#include <timeplotcomponentchannel.h>

namespace scopy {
namespace adc {
class SCOPY_ADC_EXPORT ImportChannelComponent : public ChannelComponent
{
	Q_OBJECT
public:
	ImportChannelComponent(ImportFloatChannelNode *ifcn, QPen pen, QWidget *parent = nullptr);
	~ImportChannelComponent();

	virtual void onInit() override;

public Q_SLOTS:
	void forgetChannel();

private:
	ImportFloatChannelNode *m_node;

	QVBoxLayout *m_layScroll;
	MenuPlotChannelCurveStyleControl *m_curvemenu;

	MenuPlotAxisRangeControl *m_yCtrl;
	PlotAutoscaler *m_autoscaler;
	QPushButton *m_autoscaleBtn;
	TimePlotComponentChannel *m_timePlotChannelComponent;

	bool m_yLock;

	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createYAxisMenu(QWidget *parent);
	QWidget *createCurveMenu(QWidget *parent);

	// ChannelComponent interface
public:
	void addChannelToPlot() override;
	void removeChannelFromPlot() override;
};
} // namespace adc
} // namespace scopy

#endif // IMPORTCHANNELCOMPONENT_H
