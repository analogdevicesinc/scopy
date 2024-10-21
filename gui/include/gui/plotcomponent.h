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

#ifndef PLOTCOMPONENT_H
#define PLOTCOMPONENT_H

#include "scopy-gui_export.h"
#include "cursorcontroller.h"
#include <plotwidget.h>
#include <toolcomponent.h>
#include <QWidget>
#include <QList>

namespace scopy {

class ChannelComponent;
class PlotComponent;

class SCOPY_GUI_EXPORT PlotComponentChannel
{
public:
	virtual ChannelComponent *channelComponent() = 0;
	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual void onNewData(const float *xData_, const float *yData_, size_t size, bool copy) = 0;
	virtual PlotComponent *plotComponent() = 0;
	virtual PlotChannel *plotChannel() = 0;
	virtual void initPlotComponent(PlotComponent *plotComponent) = 0;
	virtual void deinitPlotComponent() = 0;
};

class SCOPY_GUI_EXPORT PlotComponent : public QWidget, public MetaComponent
{
	Q_OBJECT
public:
	PlotComponent(QString name, uint32_t uuid, QWidget *parent = nullptr);
	~PlotComponent();

	PlotWidget *plot(int idx);
	QPair<double, double> xInterval();

public Q_SLOTS:
	virtual void replot();
	virtual void showPlotLabels(bool b);
	virtual void setName(QString s);
	virtual void refreshAxisLabels();
	virtual void selectChannel(ChannelComponent *c);
	virtual void setXInterval(double min, double max);
	virtual void setXInterval(QPair<double, double> p);
	virtual void setXUnit(QString s);

Q_SIGNALS:
	void nameChanged(QString);
	void requestDeletePlot();
	void selectComponent(PlotComponent *);

public:
	virtual void onStart();
	virtual void onStop();
	virtual void onInit();
	virtual void onDeinit();

	virtual void addChannel(ChannelComponent *);
	virtual void removeChannel(ChannelComponent *);

	uint32_t uuid();
	// TimePlotComponentSettings *createPlotMenu(QWidget *parent);
	// TimePlotComponentSettings *plotMenu();

	CursorController *cursor() const;

	QList<PlotWidget *> plots() const;

protected:
	uint32_t m_uuid;
	QHBoxLayout *m_plotLayout;
	QList<PlotWidget *> m_plots;

	QList<PlotComponentChannel *> m_channels;
	CursorController *m_cursor;
};
} // namespace scopy
#endif // PLOTCOMPONENT_H
