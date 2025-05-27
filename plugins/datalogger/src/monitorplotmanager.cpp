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

#include "monitorplotmanager.h"
#include <QVBoxLayout>
#include <plotmanagercombobox.h>
#include <QComboBox>

using namespace scopy;
using namespace datamonitor;

MonitorPlotManager::MonitorPlotManager(QString name, QWidget *parent)
	: PlotManager(name, parent)
{
	m_primary = nullptr;
}

MonitorPlotManager::~MonitorPlotManager() {}

uint32_t MonitorPlotManager::addPlot(QString name)
{
	QString plotName = name + " " + QString::number(m_plotIdx);
	MonitorPlot *plt = new MonitorPlot(plotName, m_plotIdx, this);
	m_plotIdx++;
	m_plots.append(plt);
	if(m_primary == nullptr) {
		m_primary = plt;
	}

	connect(this, &PlotManager::newData, plt->plot(), &PlotWidget::newData);

	connect(plt, &MonitorPlot::requestDeletePlot, this, [=]() {
		Q_EMIT plotRemoved(plt->uuid());
		removePlot(plt->uuid());
		delete plt;
	});

	addComponent(plt);

	connect(this, &MonitorPlotManager::requestSetStartTime, plt, &MonitorPlot::setStartTime);

	int idx = m_lay->indexOf(m_statsPanel);
	m_lay->insertWidget(idx, plt);
	for(PlotManagerCombobox *p : m_channelPlotcomboMap.values()) {
		p->addPlot(plt);
	}

	// Connect the new plot's nameChanged signal to all existing plot assignment combos
	for(QComboBox *combo : m_plotAssignmentCombos) {
		QObject::connect(plt, &MonitorPlot::nameChanged, combo, [combo, plt](const QString &newName) {
			for(int j = 0; j < combo->count(); ++j) {
				uint32_t itemUuid = combo->itemData(j).toUInt();
				if(itemUuid == plt->uuid()) {
					combo->setItemText(j, newName);
					break;
				}
			}
		});
	}

	multiPlotUpdate();
	Q_EMIT plotAdded(plt->uuid());
	return plt->uuid();
}

void MonitorPlotManager::removePlot(uint32_t uuid)
{
	PlotComponent *plt = plot(uuid);
	m_plots.removeAll(plt);
	removeComponent(plt);
	m_lay->removeWidget(plt);
	plt->deleteLater();

	for(PlotManagerCombobox *p : m_channelPlotcomboMap.values()) {
		p->removePlot(plt);
	}

	Q_EMIT plotRemoved(uuid);

	multiPlotUpdate();
}

MonitorPlot *MonitorPlotManager::plot(uint32_t uuid) { return dynamic_cast<MonitorPlot *>(PlotManager::plot(uuid)); }

MonitorPlot *MonitorPlotManager::plot() { return m_primary; }

void MonitorPlotManager::addPlotCurve(DataMonitorModel *dataMonitorModel)
{

	MonitorPlot *selectedPlot = plot(dataMonitorModel->plotUuid());
	if(selectedPlot != nullptr) {
		selectedPlot->addMonitor(dataMonitorModel);
	} else {
		m_primary->addMonitor(dataMonitorModel);
	}
}

void MonitorPlotManager::removePlotCurve(DataMonitorModel *dataMonitorModel)
{
	MonitorPlot *selectedPlot = plot(dataMonitorModel->plotUuid());
	if(selectedPlot != nullptr) {
		selectedPlot->removeMonitor(dataMonitorModel->getName());
	} else {
		m_primary->removeMonitor(dataMonitorModel->getName());
	}
}

void MonitorPlotManager::multiPlotUpdate()
{
	bool b = m_plots.count() > 1;

	for(PlotManagerCombobox *cb : qAsConst(m_channelPlotcomboMap)) {
		cb->setVisible(b);
	}
}

QList<QPair<uint32_t, QString>> MonitorPlotManager::plotList() const
{
	QList<QPair<uint32_t, QString>> result;
	for(PlotComponent *p : m_plots) {
		MonitorPlot *plt = dynamic_cast<MonitorPlot *>(p);
		if(plt) {
			result.append(qMakePair(plt->uuid(), plt->name()));
		}
	}
	return result;
}

PlotManagerCombobox *MonitorPlotManager::getPlotManagerCombobox() { return m_channelPlotcomboMap.first(); }

QComboBox *MonitorPlotManager::createPlotAssignmentComboBox(DataMonitorModel *model, QWidget *parent)
{
	QComboBox *combo = new QComboBox(parent);
	combo->setMinimumWidth(100);
	combo->setEditable(false);

	// Add to the list of all plot assignment combos
	m_plotAssignmentCombos.append(combo);
	// Remove from the list when destroyed
	QObject::connect(combo, &QObject::destroyed, this,
			 [this, combo]() { m_plotAssignmentCombos.removeAll(combo); });

	QList<QPair<uint32_t, QString>> plotList = this->plotList();
	for(const auto &pair : plotList) {
		combo->addItem(pair.second, QVariant::fromValue(pair.first));
	}
	// Set current index to the plot this channel is currently assigned to
	int idx = 0;
	for(int i = 0; i < combo->count(); ++i) {
		if(combo->itemData(i).toUInt() == model->plotUuid()) {
			idx = i;
			break;
		}
	}
	combo->setCurrentIndex(idx);

	// Plot assignment logic: when user selects a plot, move this channel to that plot
	QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), model, [=](int index) {
		uint32_t selectedUuid = combo->itemData(index).toUInt();
		// Remove from all plots first
		QList<QPair<uint32_t, QString>> allPlots = this->plotList();
		for(const auto &p : allPlots) {
			MonitorPlot *plt = this->plot(p.first);
			if(plt)
				plt->removeMonitor(model->getName());
		}
		// Add to the selected plot
		MonitorPlot *selectedPlot = this->plot(selectedUuid);
		if(selectedPlot) {
			selectedPlot->addMonitor(model);
			model->setPlotUuid(selectedUuid);
		}
	});

	// Keep plot selector in sync if plots are added/removed
	QObject::connect(this, &MonitorPlotManager::plotAdded, combo, [=](uint32_t uuid) {
		MonitorPlot *plt = this->plot(uuid);
		if(plt)
			combo->addItem(plt->name(), QVariant::fromValue(uuid));
	});
	QObject::connect(this, &MonitorPlotManager::plotRemoved, combo, [=](uint32_t uuid) {
		for(int i = 0; i < combo->count(); ++i) {
			if(combo->itemData(i).toUInt() == uuid) {
				combo->removeItem(i);
				break;
			}
		}
	});

	// Connect to plot name changes to update combobox entries
	for(int i = 0; i < plotList.size(); ++i) {
		uint32_t uuid = plotList[i].first;
		MonitorPlot *plt = this->plot(uuid);
		if(plt) {
			QObject::connect(plt, &MonitorPlot::nameChanged, combo, [combo, uuid](const QString &newName) {
				for(int j = 0; j < combo->count(); ++j) {
					uint32_t itemUuid = combo->itemData(j).toUInt();
					if(itemUuid == uuid) {
						combo->setItemText(j, newName);
						break;
					}
				}
			});
		}
	}

	return combo;
}

#include "moc_monitorplotmanager.cpp"
