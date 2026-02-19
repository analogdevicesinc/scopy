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
#include "faults/faultsgroup.h"
#include "faults/faultwidget.h"
#include "src/config/swiotconfig.h"
#include "src/config/configcontroller.h"
#include "src/config/configchannelview.h"
#include "src/config/configmodel.h"
#include <pluginbase/toolmenuentry.h>
#include <gui/mapstackedwidget.h>
#include <gui/plotchannel.h>
#include <gui/plotaxis.h>
#include <gui/widgets/menucollapsesection.h>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetgroup.h>
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
				controller->m_model->writeDevice(device);
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
				controller->m_model->writeFunction(function);
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

bool SWIOT_API::isAdPlotLabelsEnabled()
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad && ad->m_plot) {
		return ad->m_plot->showXAxisLabels();
	}
	return false;
}

void SWIOT_API::setAdMeasurementsEnabled(bool enabled)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad && ad->m_measureBtn) {
		ad->m_measureBtn->setChecked(enabled);
	}
}

bool SWIOT_API::isAdMeasurementsEnabled()
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad && ad->m_measureBtn) {
		return ad->m_measureBtn->isChecked();
	}
	return false;
}

void SWIOT_API::setAdPlotChannelEnabled(int idx, bool enabled)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad && idx >= 0 && idx < ad->m_enabledChannels.size()) {
		// Update the UI checkbox state
		QList<QAbstractButton *> buttons = ad->m_chnlsBtnGroup->buttons();
		if(idx < buttons.size()) {
			MenuControlButton *btn = dynamic_cast<MenuControlButton *>(buttons.at(idx));
			if(btn) {
				btn->checkBox()->setChecked(enabled);
			} else {
				qInfo(CAT_SWIOT_API) << "There is no channel button with index:" << idx;
			}
		}
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
			menu->m_samplingFreq->writeAsync(QString::number(frequency));
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
			menu->m_samplingFreq->readAsync();
			return menu->m_samplingFreq->getDataStrategy()->data().toInt();
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

int SWIOT_API::getAdChannelCount()
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad) {
		return ad->m_plotChnls.size();
	}
	return 0;
}

QStringList SWIOT_API::getAdChannelNames()
{
	QStringList names;
	Ad74413r *ad = getAd74413rInstrument();
	if(ad) {
		for(auto it = ad->m_plotChnls.begin(); it != ad->m_plotChnls.end(); ++it) {
			PlotChannel *plotCh = it.value();
			if(plotCh) {
				names.append(plotCh->name());
			}
		}
	}
	return names;
}

double SWIOT_API::getAdSampleRate()
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad) {
		return ad->m_currentSamplingInfo.sampleRate;
	}
	return 0.0;
}

// AD74413R widget group methods
QStringList SWIOT_API::getAdWidgetKeys()
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad && ad->m_widgetGroup) {
		return ad->m_widgetGroup->keys();
	}
	return QStringList();
}

QString SWIOT_API::getAdWidgetValue(const QString &key)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad && ad->m_widgetGroup) {
		IIOWidget *widget = ad->m_widgetGroup->get(key);
		if(widget) {
			widget->readAsync();
			return widget->getDataStrategy()->data();
		}
		qWarning(CAT_SWIOT_API) << "Widget with key" << key << "not found";
	}
	return QString();
}

void SWIOT_API::setAdWidgetValue(const QString &key, const QString &value)
{
	Ad74413r *ad = getAd74413rInstrument();
	if(ad && ad->m_widgetGroup) {
		IIOWidget *widget = ad->m_widgetGroup->get(key);
		if(widget) {
			widget->writeAsync(value);
			return;
		}
		qWarning(CAT_SWIOT_API) << "Widget with key" << key << "not found";
	}
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

int SWIOT_API::getMaxChannelCount()
{
	Max14906 *max = getMax14906Instrument();
	if(max) {
		return max->m_channelControls.size();
	}
	return 0;
}

QStringList SWIOT_API::getMaxChannelNames()
{
	QStringList names;
	Max14906 *max = getMax14906Instrument();
	if(max) {
		for(auto it = max->m_channelControls.begin(); it != max->m_channelControls.end(); ++it) {
			DioDigitalChannel *channel = it.value()->getDigitalChannel();
			if(channel) {
				names.append(channel->m_deviceName);
			}
		}
	}
	return names;
}

double SWIOT_API::getMaxXAxisMin()
{
	Max14906 *max = getMax14906Instrument();
	if(max && !max->m_channelControls.isEmpty()) {
		DioDigitalChannel *channel = max->m_channelControls.first()->getDigitalChannel();
		if(channel && channel->m_plot && channel->m_plot->xAxis()) {
			return channel->m_plot->xAxis()->min();
		}
	}
	return 0.0;
}

double SWIOT_API::getMaxXAxisMax()
{
	Max14906 *max = getMax14906Instrument();
	if(max && !max->m_channelControls.isEmpty()) {
		DioDigitalChannel *channel = max->m_channelControls.first()->getDigitalChannel();
		if(channel && channel->m_plot && channel->m_plot->xAxis()) {
			return channel->m_plot->xAxis()->max();
		}
	}
	return 0.0;
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

QList<int> SWIOT_API::getAdActiveFaultBits()
{
	QList<int> activeBits;
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_ad74413rFaultsDevice) {
		FaultsGroup *group = faults->m_faultsPage->m_ad74413rFaultsDevice->m_faultsGroup;
		if(group) {
			std::set<unsigned int> activeIndexes = group->getActiveIndexes();
			for(unsigned int idx : activeIndexes) {
				activeBits.append(static_cast<int>(idx));
			}
		}
	}
	return activeBits;
}

QList<int> SWIOT_API::getMaxActiveFaultBits()
{
	QList<int> activeBits;
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_max14906FaultsDevice) {
		FaultsGroup *group = faults->m_faultsPage->m_max14906FaultsDevice->m_faultsGroup;
		if(group) {
			std::set<unsigned int> activeIndexes = group->getActiveIndexes();
			for(unsigned int idx : activeIndexes) {
				activeBits.append(static_cast<int>(idx));
			}
		}
	}
	return activeBits;
}

QList<int> SWIOT_API::getAdSelectedFaultBits()
{
	QList<int> selectedBits;
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_ad74413rFaultsDevice) {
		FaultsGroup *group = faults->m_faultsPage->m_ad74413rFaultsDevice->m_faultsGroup;
		if(group) {
			std::set<unsigned int> selectedIndexes = group->getSelectedIndexes();
			for(unsigned int idx : selectedIndexes) {
				selectedBits.append(static_cast<int>(idx));
			}
		}
	}
	return selectedBits;
}

QList<int> SWIOT_API::getMaxSelectedFaultBits()
{
	QList<int> selectedBits;
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_max14906FaultsDevice) {
		FaultsGroup *group = faults->m_faultsPage->m_max14906FaultsDevice->m_faultsGroup;
		if(group) {
			std::set<unsigned int> selectedIndexes = group->getSelectedIndexes();
			for(unsigned int idx : selectedIndexes) {
				selectedBits.append(static_cast<int>(idx));
			}
		}
	}
	return selectedBits;
}

QList<int> SWIOT_API::getAdStoredFaultBits()
{
	QList<int> storedBits;
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_ad74413rFaultsDevice) {
		FaultsGroup *group = faults->m_faultsPage->m_ad74413rFaultsDevice->m_faultsGroup;
		if(group) {
			const QVector<FaultWidget *> &faultWidgets = group->getFaults();
			for(int i = 0; i < faultWidgets.size(); i++) {
				if(faultWidgets[i]->isStored()) {
					storedBits.append(i);
				}
			}
		}
	}
	return storedBits;
}

QList<int> SWIOT_API::getMaxStoredFaultBits()
{
	QList<int> storedBits;
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_max14906FaultsDevice) {
		FaultsGroup *group = faults->m_faultsPage->m_max14906FaultsDevice->m_faultsGroup;
		if(group) {
			const QVector<FaultWidget *> &faultWidgets = group->getFaults();
			for(int i = 0; i < faultWidgets.size(); i++) {
				if(faultWidgets[i]->isStored()) {
					storedBits.append(i);
				}
			}
		}
	}
	return storedBits;
}

void SWIOT_API::selectAdFaultBit(int bitIndex)
{
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_ad74413rFaultsDevice) {
		FaultsGroup *group = faults->m_faultsPage->m_ad74413rFaultsDevice->m_faultsGroup;
		if(group) {
			const QVector<FaultWidget *> &faultWidgets = group->getFaults();
			if(bitIndex >= 0 && bitIndex < faultWidgets.size()) {
				faultWidgets[bitIndex]->setPressed(true);
				// Emit the signal to trigger UI update (same as mouse click)
				Q_EMIT faultWidgets[bitIndex]->faultSelected(static_cast<unsigned int>(bitIndex));
			}
		}
	}
}

void SWIOT_API::selectMaxFaultBit(int bitIndex)
{
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_max14906FaultsDevice) {
		FaultsGroup *group = faults->m_faultsPage->m_max14906FaultsDevice->m_faultsGroup;
		if(group) {
			const QVector<FaultWidget *> &faultWidgets = group->getFaults();
			if(bitIndex >= 0 && bitIndex < faultWidgets.size()) {
				faultWidgets[bitIndex]->setPressed(true);
				// Emit the signal to trigger UI update (same as mouse click)
				Q_EMIT faultWidgets[bitIndex]->faultSelected(static_cast<unsigned int>(bitIndex));
			}
		}
	}
}

bool SWIOT_API::isAdFaultsExplanationEnabled()
{
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_ad74413rFaultsDevice) {
		MenuCollapseSection *section = faults->m_faultsPage->m_ad74413rFaultsDevice->m_explanationSection;
		if(section && section->header()) {
			return section->header()->isChecked();
		}
	}
	return false;
}

void SWIOT_API::setAdFaultsExplanationEnabled(bool enabled)
{
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_ad74413rFaultsDevice) {
		MenuCollapseSection *section = faults->m_faultsPage->m_ad74413rFaultsDevice->m_explanationSection;
		if(section && section->header()) {
			// Only click if state needs to change
			if(section->header()->isChecked() != enabled) {
				section->header()->click();
			}
		}
	}
}

bool SWIOT_API::isMaxFaultsExplanationEnabled()
{
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_max14906FaultsDevice) {
		MenuCollapseSection *section = faults->m_faultsPage->m_max14906FaultsDevice->m_explanationSection;
		if(section && section->header()) {
			return section->header()->isChecked();
		}
	}
	return false;
}

void SWIOT_API::setMaxFaultsExplanationEnabled(bool enabled)
{
	Faults *faults = getFaultsInstrument();
	if(faults && faults->m_faultsPage && faults->m_faultsPage->m_max14906FaultsDevice) {
		MenuCollapseSection *section = faults->m_faultsPage->m_max14906FaultsDevice->m_explanationSection;
		if(section && section->header()) {
			// Only click if state needs to change
			if(section->header()->isChecked() != enabled) {
				section->header()->click();
			}
		}
	}
}

#include "moc_swiot_api.cpp"
