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

#ifndef MONITORPLOTMANAGER_H
#define MONITORPLOTMANAGER_H

#include "monitorplot.hpp"
#include <QWidget>
#include <QStackedWidget>
#include <QList>
#include <QObject>
#include <plotmanager.h>
#include "scopy-datalogger_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGER_EXPORT MonitorPlotManager : public PlotManager
{
	Q_OBJECT
public:
	explicit MonitorPlotManager(QString name = "MonitorPlotManager", QWidget *parent = nullptr);
	~MonitorPlotManager();

	uint32_t addPlot(QString name) override;
	void removePlot(uint32_t uuid) override;
	MonitorPlot *plot(uint32_t uuid);
	MonitorPlot *plot();

	void addPlotCurve(DataMonitorModel *dataMonitorModel);
	void removePlotCurve(DataMonitorModel *dataMonitorModel);

	// Returns a list of pairs (plot uuid, plot name)
	QList<QPair<uint32_t, QString>> plotList() const;
	PlotManagerCombobox *getPlotManagerCombobox();

	/**
	 * Creates and returns a QComboBox for plot assignment for a given channel model.
	 * The combobox is populated with available plots and handles assignment logic.
	 */
	QComboBox *createPlotAssignmentComboBox(DataMonitorModel *model, QWidget *parent = nullptr);

Q_SIGNALS:
	void plotAdded(uint32_t);
	void plotRemoved(uint32_t);
	void requestSetStartTime();

private:
	MonitorPlot *m_primary;
	QList<QComboBox *> m_plotAssignmentCombos;
	void multiPlotUpdate();
};

} // namespace datamonitor
} // namespace scopy

#endif // MONITORPLOTMANAGER_H
