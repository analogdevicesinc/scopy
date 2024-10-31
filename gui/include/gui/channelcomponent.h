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

#ifndef CHANNELCOMPONENT_H
#define CHANNELCOMPONENT_H

#include <scopy-gui_export.h>
#include <QWidget>
#include "menucontrolbutton.h"
#include "menuwidget.h"
#include "toolcomponent.h"
#include "plotcomponent.h"
#include "interfaces.h"

namespace scopy {

class TimePlotComponentChannel;

class SCOPY_GUI_EXPORT ChannelComponent : public QWidget,
					  public ToolComponent,
					  public Menu,
					  public SamplingInfoComponent,
					  public DataProcessor
{
	Q_OBJECT
public:
	ChannelComponent(QString ch, QPen pen, QWidget *parent = nullptr);
	virtual ~ChannelComponent();

	QPen pen() const;
	ChannelData *chData() const;

	PlotComponentChannel *plotChannelCmpt() const;
	void setPlotChannelCmpt(PlotComponentChannel *newPlotChannelCmpt);

	virtual MenuControlButton *ctrl();
	virtual void addChannelToPlot();
	virtual void removeChannelFromPlot();
	virtual void setMenuControlButtonVisible(bool b);

	MenuWidget *menu() override;
	static void createMenuControlButton(ChannelComponent *c, QWidget *parent = nullptr);

	virtual SamplingInfo samplingInfo() override;
	virtual void setSamplingInfo(SamplingInfo p) override;

Q_SIGNALS:
	void requestChannelMenu(bool force = true);

protected:
	QString m_channelName;
	QPen m_pen;
	QWidget *widget;
	MenuControlButton *m_ctrl;
	MenuWidget *m_menu;

	ChannelData *m_chData;

	PlotComponentChannel *m_plotChannelCmpt;
	DataProcessor *m_dataProcessor;

	SamplingInfo m_samplingInfo;

	void initMenu(QWidget *parent = nullptr);

public Q_SLOTS:
	virtual void enable() override;
	virtual void disable() override;
	virtual void onStart() override;
	virtual void onStop() override;
	virtual void onInit() override;
	virtual void onDeinit() override;
	virtual void onNewData(const float *xData, const float *yData, size_t size, bool copy) override;
};

} // namespace scopy
#endif // CHANNELCOMPONENT_H
