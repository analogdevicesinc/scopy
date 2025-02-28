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

#include "adcinstrumentcontroller.h"
#include "timeplotcomponent.h"
#include "adcinstrument.h"

#include <pluginbase/preferences.h>
#include "adcinterfaces.h"

using namespace scopy;
using namespace scopy::adc;

ADCInstrumentController::ADCInstrumentController(ToolMenuEntry *tme, QString uri, QString name, AcqTreeNode *tree,
						 QObject *parent)
	: QObject(parent)
	, m_refreshTimerRunning(false)
	, m_plotComponentManager(nullptr)
	, m_measureComponent(nullptr)
	, m_started(false)
	, m_tme(tme)
	, m_uri(uri)
{
	chIdP = new ChannelIdProvider(this);
	m_tree = tree;
	m_name = name;

	Preferences *p = Preferences::GetInstance();

	m_plotTimer = new QTimer(this);
	m_plotTimer->setSingleShot(true);
	connect(m_plotTimer, &QTimer::timeout, this, &ADCInstrumentController::updateData);
	connect(p, SIGNAL(preferenceChanged(QString, QVariant)), this, SLOT(handlePreferences(QString, QVariant)));

	m_fw = new QFutureWatcher<void>(this);
	connect(
		m_fw, &QFutureWatcher<void>::finished, this,
		[=]() {
			update();
			if(m_refreshTimerRunning)
				m_plotTimer->start();
		},
		Qt::QueuedConnection);

	m_ui = new ADCInstrument(tme, nullptr);
}

ADCInstrumentController::~ADCInstrumentController() {}

ChannelIdProvider *ADCInstrumentController::getChannelIdProvider() { return chIdP; }

void ADCInstrumentController::setEnableAddRemovePlot(bool) {}

void ADCInstrumentController::setEnableAddRemoveInstrument(bool b)
{
	m_ui->addBtn->setVisible(b);
	if(!m_isMainInstrument) {
		m_ui->removeBtn->setVisible(b);
	}
}

void ADCInstrumentController::init() {}

void ADCInstrumentController::deinit()
{
	for(auto c : qAsConst(m_components)) {
		c->onDeinit();
	}
}

void ADCInstrumentController::onStart()
{
	for(auto c : qAsConst(m_components)) {
		if(c->enabled()) {
			c->onStart();
		}
	}
	m_started = true;
}

void ADCInstrumentController::onStop()
{
	m_started = false;
	for(int idx = m_components.size() - 1; idx >= 0; idx--) {
		auto c = m_components[idx];
		c->onStop();
	}
}

void ADCInstrumentController::start()
{
	ResourceManager::open("adc" + m_uri, this);
	bool ret = m_dataProvider->start();
	if(!ret) {
		Q_EMIT requestDisconnect();
	}
}

void ADCInstrumentController::stop()
{
	m_dataProvider->stop();
	ResourceManager::close("adc" + m_uri);
}

void ADCInstrumentController::stopUpdates()
{
	m_refreshTimerRunning = false;
	m_refillFuture.cancel();
	m_plotTimer->stop();
	m_ui->stopped();
}

void ADCInstrumentController::startUpdates()
{
	updateFrameRate();
	m_refreshTimerRunning = true;
	update();
	m_plotTimer->start();
	m_ui->started();
}

void ADCInstrumentController::setSingleShot(bool b) { m_dataProvider->setSingleShot(b); }

void ADCInstrumentController::updateData()
{
	m_refillFuture = QtConcurrent::run([=]() { m_dataProvider->updateData(); });
	m_fw->setFuture(m_refillFuture);
}

void ADCInstrumentController::update()
{
	m_dataProvider->setData(false);
	if(m_dataProvider->finished()) {
		Q_EMIT requestStop();
	}
	m_plotComponentManager->replot();
}

void ADCInstrumentController::handlePreferences(QString key, QVariant v)
{
	if(key == "general_plot_target_fps") {
		updateFrameRate();
	}
}

void ADCInstrumentController::updateFrameRate()
{
	Preferences *p = Preferences::GetInstance();
	double framerate = p->get("general_plot_target_fps").toDouble();
	setFrameRate(framerate);
}

void ADCInstrumentController::setFrameRate(double val)
{
	int timeout = (1.0 / val) * 1000;
	m_plotTimer->setInterval(timeout);
}

/*void ADCInstrumentController::addChannel(AcqTreeNode *node)
{

}*/

/*void ADCInstrumentController::removeChannel(AcqTreeNode *node)
{

}*/

void ADCInstrumentController::setupChannelMeasurement(PlotManager *c, ChannelComponent *ch)
{
	auto chMeasureableChannel = dynamic_cast<MeasurementProvider *>(ch);
	if(!chMeasureableChannel)
		return;
	auto chMeasureManager = chMeasureableChannel->getMeasureManager();
	if(!chMeasureManager)
		return;
	if(m_measureComponent) {
		auto measureSettings = m_measureComponent->measureSettings();
		auto measurePanel = c->measurePanel();
		auto statsPanel = c->statsPanel();
		connect(chMeasureManager, &MeasureManagerInterface::enableMeasurement, measurePanel,
			&MeasurementsPanel::addMeasurement);
		connect(chMeasureManager, &MeasureManagerInterface::disableMeasurement, measurePanel,
			&MeasurementsPanel::removeMeasurement);
		connect(measureSettings, &MeasurementSettings::toggleAllMeasurements, this, [=](bool b) {
			measurePanel->setInhibitUpdates(true);
			Q_EMIT chMeasureManager->toggleAllMeasurement(b);
			measurePanel->setInhibitUpdates(false);
		});
		connect(measureSettings, &MeasurementSettings::toggleAllStats, this,
			[=](bool b) { Q_EMIT chMeasureManager->toggleAllStats(b); });
		connect(chMeasureManager, &MeasureManagerInterface::enableStat, statsPanel, &StatsPanel::addStat);
		connect(chMeasureManager, &MeasureManagerInterface::disableStat, statsPanel, &StatsPanel::removeStat);
	}
}

bool ADCInstrumentController::isMainInstrument() const { return m_isMainInstrument; }

void ADCInstrumentController::setIsMainInstrument(bool newIsMainInstrument)
{
	m_isMainInstrument = newIsMainInstrument;
	m_ui->removeBtn->setVisible(!newIsMainInstrument);
}

ADCInstrument *ADCInstrumentController::ui() const { return m_ui; }
