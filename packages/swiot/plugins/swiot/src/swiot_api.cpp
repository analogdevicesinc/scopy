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

#include "swiot_api.h"
#include "swiotplugin.h"
#include "ad74413r/ad74413r.h"
#include "ad74413r/buffermenuview.h"
#include "ad74413r/buffermenu.h"
#include "max14906/max14906.h"
#include "max14906/diodigitalchannel.h"
#include "max14906/diosettingstab.h"
#include "faults/faults.h"
#include "faults/faultspage.h"
#include "faults/faultsdevice.h"
#include "src/config/swiotconfig.h"
#include "src/config/configcontroller.h"
#include "src/config/configchannelview.h"
#include <pluginbase/toolmenuentry.h>
#include <gui/mapstackedwidget.h>
#include <gui/plotchannel.h>
#include <gui/plotaxis.h>
#include <iio-widgets/iiowidget.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_SWIOT_API, "SWIOT_API")

using namespace scopy::swiot;

SWIOT_API::SWIOT_API(SWIOTPlugin *swiotPlugin)
	: ApiObject()
	, m_swiotPlugin(swiotPlugin)
{}

SWIOT_API::~SWIOT_API() {}

SwiotConfig *SWIOT_API::getConfigInstrument()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_swiotPlugin->m_toolList, CONFIG_TME_ID);
	if(tool && tool->tool()) {
		return dynamic_cast<SwiotConfig *>(tool->tool());
	}
	return nullptr;
}

Ad74413r *SWIOT_API::getAd74413rInstrument()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_swiotPlugin->m_toolList, AD74413R_TME_ID);
	if(tool && tool->tool()) {
		return dynamic_cast<Ad74413r *>(tool->tool());
	}
	return nullptr;
}

Max14906 *SWIOT_API::getMax14906Instrument()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_swiotPlugin->m_toolList, MAX14906_TME_ID);
	if(tool && tool->tool()) {
		return dynamic_cast<Max14906 *>(tool->tool());
	}
	return nullptr;
}

Faults *SWIOT_API::getFaultsInstrument()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_swiotPlugin->m_toolList, FAULTS_TME_ID);
	if(tool && tool->tool()) {
		return dynamic_cast<Faults *>(tool->tool());
	}
	return nullptr;
}

QStringList SWIOT_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_swiotPlugin->m_toolList) {
		if(tool->visible()) {
			tools.append(tool->name());
		}
	}
	return tools;
}

bool SWIOT_API::isRuntimeMode() { return m_swiotPlugin->m_isRuntime; }

void SWIOT_API::switchToConfigMode()
{
	if(!m_swiotPlugin->m_isRuntime) {
		qWarning(CAT_SWIOT_API) << "Already in config mode";
		return;
	}
	// Trigger config button from any runtime instrument
	Ad74413r *ad = getAd74413rInstrument();
	if(ad) {
		Q_EMIT ad->configBtnPressed();
	}
}

// Config instrument methods
int SWIOT_API::getConfigChannelCount()
{
	SwiotConfig *config = getConfigInstrument();
	if(config) {
		return config->m_controllers.size();
	}
	return 0;
}

// works
void SWIOT_API::applyConfig()
{
	SwiotConfig *config = getConfigInstrument();
	if(config) {
		config->onConfigBtnPressed();
	}
}

// works
void SWIOT_API::setChannelEnabled(int channelId, bool enabled)
{
	SwiotConfig *config = getConfigInstrument();
	if(config && channelId >= 0 && channelId < config->m_controllers.size()) {
		ConfigController *controller = config->m_controllers[channelId];
		if(controller && controller->m_channelsView) {
			controller->m_channelsView->setChannelEnabled(enabled);
		}
	}
}

// works
bool SWIOT_API::isChannelEnabled(int channelId)
{
	SwiotConfig *config = getConfigInstrument();
	if(config && channelId >= 0 && channelId < config->m_controllers.size()) {
		ConfigController *controller = config->m_controllers[channelId];
		if(controller && controller->m_channelsView) {
			return controller->m_channelsView->isChannelEnabled();
		}
	}
	return false;
}

// works
void SWIOT_API::setChannelDevice(int channelId, const QString &device)
{
	SwiotConfig *config = getConfigInstrument();
	if(config && channelId >= 0 && channelId < config->m_controllers.size()) {
		ConfigController *controller = config->m_controllers[channelId];
		if(controller && controller->m_channelsView && controller->m_channelsView->isChannelEnabled()) {
			QStringList availableDevices = controller->m_channelsView->getDeviceAvailable();
			if(availableDevices.contains(device)) {
				controller->m_channelsView->setSelectedDevice(device);
			} else {
				qWarning(CAT_SWIOT_API) << "Device" << device << "not available for channel"
							<< channelId << ". Available:" << availableDevices;
			}
		}
	}
}

// works
QString SWIOT_API::getChannelDevice(int channelId)
{
	SwiotConfig *config = getConfigInstrument();
	if(config && channelId >= 0 && channelId < config->m_controllers.size()) {
		ConfigController *controller = config->m_controllers[channelId];
		if(controller && controller->m_channelsView) {
			return controller->m_channelsView->getSelectedDevice();
		}
	}
	return QString();
}

// works
void SWIOT_API::setChannelFunction(int channelId, const QString &function)
{
	SwiotConfig *config = getConfigInstrument();
	if(config && channelId >= 0 && channelId < config->m_controllers.size()) {
		ConfigController *controller = config->m_controllers[channelId];
		if(controller && controller->m_channelsView && controller->m_channelsView->isChannelEnabled()) {
			QStringList availableFunctions = controller->m_channelsView->getFunctionAvailable();
			if(availableFunctions.contains(function)) {
				controller->m_channelsView->setSelectedFunction(function);
			} else {
				qWarning(CAT_SWIOT_API) << "Function" << function << "not available for channel"
							<< channelId << ". Available:" << availableFunctions;
			}
		}
	}
}

// works
QString SWIOT_API::getChannelFunction(int channelId)
{
	SwiotConfig *config = getConfigInstrument();
	if(config && channelId >= 0 && channelId < config->m_controllers.size()) {
		ConfigController *controller = config->m_controllers[channelId];
		if(controller && controller->m_channelsView) {
			return controller->m_channelsView->getSelectedFunction();
		}
	}
	return QString();
}

QStringList SWIOT_API::getAvailableDevices(int channelId)
{
	SwiotConfig *config = getConfigInstrument();
	if(config && channelId >= 0 && channelId < config->m_controllers.size()) {
		ConfigController *controller = config->m_controllers[channelId];
		if(controller && controller->m_channelsView) {
			return controller->m_channelsView->getDeviceAvailable();
		}
	}
	return QStringList();
}

QStringList SWIOT_API::getAvailableFunctions(int channelId)
{
	SwiotConfig *config = getConfigInstrument();
	if(config && channelId >= 0 && channelId < config->m_controllers.size()) {
		ConfigController *controller = config->m_controllers[channelId];
		if(controller && controller->m_channelsView) {
			return controller->m_channelsView->getFunctionAvailable();
		}
	}
	return QStringList();
}

// AD74413R instrument methods
// works
bool SWIOT_API::isAdRunning()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_swiotPlugin->m_toolList, AD74413R_TME_ID);
	return tool ? tool->running() : false;
}

// works
void SWIOT_API::setAdRunning(bool running)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad && ad->m_runBtn && ad->m_runBtn->isEnabled()) {
		ad->m_runBtn->setChecked(running);
	}
}

// works
void SWIOT_API::adSingleShot()
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad && ad->m_singleBtn && ad->m_singleBtn->isEnabled()) {
		ad->m_singleBtn->setChecked(true);
	}
}

// works
double SWIOT_API::getAdTimespan()
{
	Ad74413r *ad = getAd74413rInstrument();
	return ad && ad->m_timespanSpin ? ad->m_timespanSpin->value() : 1.0;
}

// works
void SWIOT_API::setAdTimespan(double timespan)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad && ad->m_timespanSpin) {
		ad->m_timespanSpin->setValue(timespan);
	}
}

// works
void SWIOT_API::setAdPlotLabelsEnabled(bool enabled)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad) {
		ad->showPlotLabels(enabled);
	}
}

// changes must be made. The UI doesn't update when a channel is enabled
void SWIOT_API::setAdChannelEnabled(int channelId, bool enabled)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad && channelId >= 0 && channelId < ad->m_enabledChannels.size()) {
		ad->onChannelBtnChecked(channelId, enabled);
	}
}

// works
bool SWIOT_API::isAdChannelEnabled(int channelId)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad && channelId >= 0 && channelId < ad->m_enabledChannels.size()) {
		return ad->m_enabledChannels[channelId];
	}
	return false;
}

void SWIOT_API::setAdChannelSamplingFrequency(int channelId, int frequency)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(!ad || !ad->m_plotChnls.contains(channelId)) {
		qWarning(CAT_SWIOT_API) << "Channel" << channelId << "not found";
		return;
	}

	PlotChannel *plotCh = ad->m_plotChnls[channelId];
	QString chnlName = plotCh->name();
	QWidget *widget = ad->m_channelStack->get(chnlName);
	BufferMenuView *menuView = dynamic_cast<BufferMenuView *>(widget);
	if(menuView) {
		BufferMenu *menu = menuView->getAdvMenu();
		if(menu && menu->m_samplingFreq) {
			menu->m_samplingFreq->getDataStrategy()->write(QString::number(frequency));
		}
	}
}

int SWIOT_API::getAdChannelSamplingFrequency(int channelId)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(!ad || !ad->m_plotChnls.contains(channelId)) {
		qWarning(CAT_SWIOT_API) << "Channel" << channelId << "not found";
		return -1;
	}

	PlotChannel *plotCh = ad->m_plotChnls[channelId];
	QString chnlName = plotCh->name();
	QWidget *widget = ad->m_channelStack->get(chnlName);
	BufferMenuView *menuView = dynamic_cast<BufferMenuView *>(widget);
	if(menuView) {
		BufferMenu *menu = menuView->getAdvMenu();
		if(menu && menu->m_samplingFreq) {
			QString data = menu->m_samplingFreq->getDataStrategy()->data();
			return data.toInt();
		}
	}
	return -1;
}

void SWIOT_API::setAdChannelYMin(int channelId, double value)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(!ad || !ad->m_plotChnls.contains(channelId)) {
		qWarning(CAT_SWIOT_API) << "Channel" << channelId << "not found";
		return;
	}

	PlotChannel *plotCh = ad->m_plotChnls[channelId];
	if(plotCh && plotCh->yAxis()) {
		plotCh->yAxis()->setMin(value);
	}
}

void SWIOT_API::setAdChannelYMax(int channelId, double value)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(!ad || !ad->m_plotChnls.contains(channelId)) {
		qWarning(CAT_SWIOT_API) << "Channel" << channelId << "not found";
		return;
	}

	PlotChannel *plotCh = ad->m_plotChnls[channelId];
	if(plotCh && plotCh->yAxis()) {
		plotCh->yAxis()->setMax(value);
	}
}

double SWIOT_API::getAdChannelYMin(int channelId)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(!ad || !ad->m_plotChnls.contains(channelId)) {
		qWarning(CAT_SWIOT_API) << "Channel" << channelId << "not found";
		return 0.0;
	}

	PlotChannel *plotCh = ad->m_plotChnls[channelId];
	if(plotCh && plotCh->yAxis()) {
		return plotCh->yAxis()->min();
	}
	return 0.0;
}

double SWIOT_API::getAdChannelYMax(int channelId)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(!ad || !ad->m_plotChnls.contains(channelId)) {
		qWarning(CAT_SWIOT_API) << "Channel" << channelId << "not found";
		return 0.0;
	}

	PlotChannel *plotCh = ad->m_plotChnls[channelId];
	if(plotCh && plotCh->yAxis()) {
		return plotCh->yAxis()->max();
	}
	return 0.0;
}

// MAX14906 instrument methods
bool SWIOT_API::isMaxRunning()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_swiotPlugin->m_toolList, MAX14906_TME_ID);
	return tool ? tool->running() : false;
}

void SWIOT_API::setMaxRunning(bool running)
{
	Max14906 *max = getMax14906Instrument();
	if(max && max->m_runBtn) {
		max->m_runBtn->setChecked(running);
	}
}

double SWIOT_API::getMaxTimespan()
{
	Max14906 *max = getMax14906Instrument();
	if(max && max->m_max14906SettingsTab) {
		return max->m_max14906SettingsTab->getTimeValue();
	}
	return 1.0;
}

void SWIOT_API::setMaxTimespan(double timespan)
{
	Max14906 *max = getMax14906Instrument();
	if(max && max->m_max14906SettingsTab) {
		max->m_max14906SettingsTab->m_maxSpinButton->setValue(timespan);
	}
}

void SWIOT_API::setMaxChannelOutput(int channelId, bool value)
{
	Max14906 *max = getMax14906Instrument();
	if(max && max->m_channelControls.contains(channelId)) {
		DioDigitalChannel *channel = max->m_channelControls[channelId]->getDigitalChannel();
		if(channel && channel->m_valueSwitch) {
			channel->m_valueSwitch->setChecked(value);
		}
	}
}

bool SWIOT_API::getMaxChannelOutput(int channelId)
{
	Max14906 *max = getMax14906Instrument();
	if(max && max->m_channelControls.contains(channelId)) {
		DioDigitalChannel *channel = max->m_channelControls[channelId]->getDigitalChannel();
		if(channel && channel->m_valueSwitch) {
			return channel->m_valueSwitch->isChecked();
		}
	}
	return false;
}

// Faults instrument methods
bool SWIOT_API::isFaultsRunning()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_swiotPlugin->m_toolList, FAULTS_TME_ID);
	return tool ? tool->running() : false;
}

void SWIOT_API::setFaultsRunning(bool running)
{
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_runBtn) {
		faults->m_runBtn->setChecked(running);
	}
}

void SWIOT_API::faultsSingleShot()
{
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_singleBtn) {
		Q_EMIT faults->m_singleBtn->clicked(true);
	}
}

void SWIOT_API::resetAdFaultsStored()
{
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_ad74413rFaultsDevice) {
		faults->m_faultsPage->m_ad74413rFaultsDevice->resetStored();
	}
}

void SWIOT_API::resetMaxFaultsStored()
{
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_max14906FaultsDevice) {
		faults->m_faultsPage->m_max14906FaultsDevice->resetStored();
	}
}

void SWIOT_API::clearAdFaultsSelection()
{
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_ad74413rFaultsDevice &&
	   faults->m_faultsPage->m_ad74413rFaultsDevice->m_clearBtn) {
		faults->m_faultsPage->m_ad74413rFaultsDevice->m_clearBtn->click();
	}
}

void SWIOT_API::clearMaxFaultsSelection()
{
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_max14906FaultsDevice &&
	   faults->m_faultsPage->m_max14906FaultsDevice->m_clearBtn) {
		faults->m_faultsPage->m_max14906FaultsDevice->m_clearBtn->click();
	}
}

#include "moc_swiot_api.cpp"
