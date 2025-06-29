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

#ifndef TIMEPLOTCOMPONENT_H
#define TIMEPLOTCOMPONENT_H

#include "scopy-adc_export.h"

#include <QFuture>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <QVariant>

#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menucombo.h>
#include <gui/plotautoscaler.h>

#include <plotwidget.h>
#include "plotinfo.h"
#include <gui/plotcomponent.h>
#include <gui/widgets/plotinfowidgets.h>
#include <gui/docking/dockwrapperinterface.h>
#include <gui/docking/dockableareainterface.h>

namespace scopy {
namespace adc {

class TimePlotComponentSettings;
class TimePlotComponentChannel;

class SCOPY_ADC_EXPORT TimePlotComponent : public PlotComponent
{
	Q_OBJECT
public:
	TimePlotComponent(QString name, uint32_t uuid, QWidget *parent = nullptr);
	~TimePlotComponent();

	virtual PlotWidget *timePlot();
	virtual PlotWidget *xyPlot();

public Q_SLOTS:
	void setSingleYModeAll(bool b);
	void showXSourceOnXy(bool b);

	ChannelComponent *XYXChannel();
	void setXYXChannel(ChannelComponent *c);
	void refreshXYXAxis();
	void refreshXYXData();
	void selectChannel(ChannelComponent *c) override;
	void setXInterval(QPair<double, double> p) override;
	void setXInterval(double min, double max) override;

public:
	void addChannel(ChannelComponent *) override;
	void removeChannel(ChannelComponent *) override;

	TimePlotComponentSettings *createPlotMenu(QWidget *parent);
	TimePlotComponentSettings *plotMenu();

	DockWrapperInterface *timeDockWidget() const;
	DockWrapperInterface *xyDockWidget() const;

	bool singleYMode() const;

	TimeSamplingInfo *timePlotInfo() const;

private Q_SLOTS:
	void onXyXNewData(const float *xData_, const float *yData_, size_t size, bool copy);

private:
	PlotWidget *m_timePlot;
	PlotWidget *m_xyPlot;
	PlotInfo *m_timeInfo;
	PlotInfo *m_xyInfo;

	TimePlotComponentSettings *m_plotMenu;

	bool m_singleYMode;
	bool m_showXSourceOnXy;

	ChannelComponent *m_XYXChannel;
	TimeSamplingInfo *m_timePlotInfo;
	const float *xyXData;

	DockableAreaInterface *m_dockableArea;
	DockWrapperInterface *m_timeDockWidget;
	DockWrapperInterface *m_xyDockWidget;

private:
	QMetaObject::Connection xyDataConn;
	QMetaObject::Connection xyAxisMinConn;
	QMetaObject::Connection xyAxisMaxConn;
};

} // namespace adc
} // namespace scopy

#endif // TIMEPLOTCOMPONENT_H
