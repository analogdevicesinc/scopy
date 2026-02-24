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

#ifndef PLOT_API_H
#define PLOT_API_H

#include "scopy-gui_export.h"

#include <pluginbase/apiobject.h>
#include <QStringList>

namespace scopy {

class PlotComponent;
class PlotWidget;
class PlotChannel;
class CursorController;

class SCOPY_GUI_EXPORT PlotAPI : public ApiObject
{
	Q_OBJECT
public:
	explicit PlotAPI(PlotComponent *component, PlotWidget *plot,
			 CursorController *cursor = nullptr, QObject *parent = nullptr);
	~PlotAPI();

	// Title & Labels
	Q_INVOKABLE QString getTitle();
	Q_INVOKABLE void setTitle(const QString &title);
	Q_INVOKABLE bool isLabelsEnabled();
	Q_INVOKABLE void setLabelsEnabled(bool enabled);

	// Channels
	Q_INVOKABLE QStringList getChannels();

	// Curve style (per channel by name)
	Q_INVOKABLE int getChannelThickness(const QString &channel);
	Q_INVOKABLE void setChannelThickness(const QString &channel, int thickness);
	Q_INVOKABLE int getChannelStyle(const QString &channel);
	Q_INVOKABLE void setChannelStyle(const QString &channel, int style);

	// X-Axis range
	Q_INVOKABLE double getXMin();
	Q_INVOKABLE double getXMax();
	Q_INVOKABLE void setXMin(double min);
	Q_INVOKABLE void setXMax(double max);

	// Y-Axis range (main/global axis)
	Q_INVOKABLE double getYMin();
	Q_INVOKABLE double getYMax();
	Q_INVOKABLE void setYMin(double min);
	Q_INVOKABLE void setYMax(double max);

	// Cursors
	Q_INVOKABLE bool isCursorsVisible();
	Q_INVOKABLE void setCursorsVisible(bool visible);
	Q_INVOKABLE void setXCursorsEnabled(bool enabled);
	Q_INVOKABLE void setYCursorsEnabled(bool enabled);
	Q_INVOKABLE void setXCursorsLocked(bool locked);
	Q_INVOKABLE void setYCursorsLocked(bool locked);
	Q_INVOKABLE void setTrackingEnabled(bool enabled);
	Q_INVOKABLE double getX1CursorPosition();
	Q_INVOKABLE double getX2CursorPosition();
	Q_INVOKABLE void setX1CursorPosition(double pos);
	Q_INVOKABLE void setX2CursorPosition(double pos);
	Q_INVOKABLE double getY1CursorPosition();
	Q_INVOKABLE double getY2CursorPosition();
	Q_INVOKABLE void setY1CursorPosition(double pos);
	Q_INVOKABLE void setY2CursorPosition(double pos);

	// Print/Export
	Q_INVOKABLE bool printPlot(const QString &filePath);

private:
	PlotComponent *m_component;
	PlotWidget *m_plot;
	CursorController *m_cursor;

	PlotChannel *findChannel(const QString &name);
};

} // namespace scopy

#endif // PLOT_API_H
