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

#ifndef PLOTMANAGER_H
#define PLOTMANAGER_H

#include "scopy-gui_export.h"
#include <QWidget>
#include "measurementpanel.h"
#include "channelcomponent.h"
#include "toolcomponent.h"
#include "plotmarkercontroller.h"

namespace scopy {

class PlotManagerCombobox;
class SCOPY_GUI_EXPORT PlotManager : public QWidget, public MeasurementPanelInterface, public MetaComponent
{
	Q_OBJECT
public:
	PlotManager(QString name = "PlotManager", QWidget *parent = nullptr);
	~PlotManager();

	virtual uint32_t addPlot(QString name) = 0;
	virtual void removePlot(uint32_t uuid) = 0;

	virtual void addChannel(ChannelComponent *);
	virtual void moveChannel(ChannelComponent *, uint32_t uuid = 0);
	virtual void removeChannel(ChannelComponent *);

	// TimePlotComponent* plot(QString name);
	PlotComponent *plot(uint32_t uuid);

	QList<PlotComponent *> plots() const;
	MeasurementsPanel *measurePanel() const override;
	StatsPanel *statsPanel() const override;
	MarkerPanel *markerPanel() const override;

	QWidget *createMenu(QWidget *parent);
	QWidget *plotCombo(ChannelComponent *c);

public Q_SLOTS:
	void updateAxisScales();
	void replot();
	void enableMeasurementPanel(bool) override;
	void enableStatsPanel(bool) override;
	void enableMarkerPanel(bool) override;

	void setXInterval(double xMin, double xMax);
	void setXUnit(QString);
	void selectChannel(ChannelComponent *c);
Q_SIGNALS:
	void plotAdded(uint32_t);
	void plotRemoved(uint32_t);
	void newData();

protected:
	uint32_t m_plotIdx;
	QVBoxLayout *m_lay;
	QPair<double, double> m_xInterval;
	QList<PlotComponent *> m_plots;
	QList<PlotComponentChannel *> m_channels;
	MeasurementsPanel *m_measurePanel;
	MarkerPanel *m_markerPanel;

	StatsPanel *m_statsPanel;
	QMap<ChannelComponent *, PlotManagerCombobox *> m_channelPlotcomboMap;
	// PlotSettings *m_plotSettings;
};
} // namespace scopy
#endif // PLOTMANAGER_H
