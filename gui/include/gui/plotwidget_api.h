/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef PLOTWIDGET_API_H
#define PLOTWIDGET_API_H

#include "scopy-gui_export.h"

#include <pluginbase/apiobject.h>
#include <QStringList>

namespace scopy {

class PlotWidget;
class PlotChannel;

class SCOPY_GUI_EXPORT PlotWidget_API : public ApiObject
{
	Q_OBJECT
public:
	explicit PlotWidget_API(PlotWidget *plot, QObject *parent = nullptr);
	~PlotWidget_API();

	// X-Axis
	Q_INVOKABLE double getXMin();
	Q_INVOKABLE double getXMax();
	Q_INVOKABLE void setXMin(double val);
	Q_INVOKABLE void setXMax(double val);
	Q_INVOKABLE void setXInterval(double min, double max);
	Q_INVOKABLE QString getXUnits();
	Q_INVOKABLE void setXUnits(const QString &units);
	Q_INVOKABLE double getXDivs();
	Q_INVOKABLE void setXDivs(double divs);

	// Y-Axis
	Q_INVOKABLE double getYMin();
	Q_INVOKABLE double getYMax();
	Q_INVOKABLE void setYMin(double val);
	Q_INVOKABLE void setYMax(double val);
	Q_INVOKABLE void setYInterval(double min, double max);
	Q_INVOKABLE QString getYUnits();
	Q_INVOKABLE void setYUnits(const QString &units);
	Q_INVOKABLE double getYDivs();
	Q_INVOKABLE void setYDivs(double divs);

	// Channels
	Q_INVOKABLE QStringList getChannelNames();
	Q_INVOKABLE void setChannelEnabled(const QString &name, bool enabled);
	Q_INVOKABLE bool isChannelEnabled(const QString &name);
	Q_INVOKABLE void setChannelThickness(const QString &name, int thickness);
	Q_INVOKABLE int getChannelThickness(const QString &name);
	Q_INVOKABLE void setChannelStyle(const QString &name, int style);
	Q_INVOKABLE int getChannelStyle(const QString &name);
	Q_INVOKABLE void selectChannel(const QString &name);

	// Grid/Scales
	Q_INVOKABLE void setGridEnabled(bool enabled);
	Q_INVOKABLE bool isGridEnabled();
	Q_INVOKABLE void setGraticuleEnabled(bool enabled);
	Q_INVOKABLE bool isGraticuleEnabled();

	// Labels
	Q_INVOKABLE void setShowXAxisLabels(bool show);
	Q_INVOKABLE void setShowYAxisLabels(bool show);

	// Navigator
	Q_INVOKABLE void setNavigatorEnabled(bool enabled);
	Q_INVOKABLE bool isNavigatorEnabled();

	// General
	Q_INVOKABLE void replot();
	Q_INVOKABLE QString exportCsv();

private:
	PlotChannel *findChannel(const QString &name);

	PlotWidget *m_plot;
};

} // namespace scopy

#endif // PLOTWIDGET_API_H
