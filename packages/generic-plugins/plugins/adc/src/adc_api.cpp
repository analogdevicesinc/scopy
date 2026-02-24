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

#include "adc_api.h"
#include "adcplugin.h"
#include "adcinstrument.h"
#include "adcinstrumentcontroller.h"
#include "adctimeinstrumentcontroller.h"
#include "adcfftinstrumentcontroller.h"
#include "grtimechannelcomponent.h"
#include "freq/grfftchannelcomponent.h"
#include "timeplotmanagersettings.h"
#include "freq/fftplotmanagersettings.h"
#include "timeplotcomponent.h"
#include "freq/fftplotcomponent.h"
#include "timeplotcomponentchannel.h"
#include "freq/fftplotcomponentchannel.h"
#include "timeplotcomponentsettings.h"
#include "freq/fftplotcomponentsettings.h"
#include "measurecomponent.h"
#include "genalyzersettings.h"
#include "measurementcontroller.h"

#include <pluginbase/toolmenuentry.h>
#include <gui/plotaxis.h>
#include <gui/plotmarkercontroller.h>
#include <gui/cursorcontroller.h>
#include <gui/widgets/measurementsettings.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_ADC_API, "ADC_API")

using namespace scopy;
using namespace scopy::adc;

ADC_API::ADC_API(ADCPlugin *adcPlugin)
	: ApiObject()
	, m_adcPlugin(adcPlugin)
{}

ADC_API::~ADC_API() {}

// ==================== HELPERS ====================

ADCTimeInstrumentController *ADC_API::getTimeController()
{
	for(auto *ctrl : m_adcPlugin->m_ctrls) {
		auto *timeCtrl = dynamic_cast<ADCTimeInstrumentController *>(ctrl);
		if(timeCtrl)
			return timeCtrl;
	}
	return nullptr;
}

ADCFFTInstrumentController *ADC_API::getFreqController()
{
	for(auto *ctrl : m_adcPlugin->m_ctrls) {
		auto *fftCtrl = dynamic_cast<ADCFFTInstrumentController *>(ctrl);
		if(fftCtrl)
			return fftCtrl;
	}
	return nullptr;
}

GRTimeChannelComponent *ADC_API::findTimeChannel(const QString &name)
{
	auto *ctrl = getTimeController();
	if(!ctrl)
		return nullptr;
	for(auto it = ctrl->m_acqNodeComponentMap.begin(); it != ctrl->m_acqNodeComponentMap.end(); ++it) {
		auto *ch = dynamic_cast<GRTimeChannelComponent *>(it.value());
		if(ch && ch->name() == name)
			return ch;
	}
	return nullptr;
}

GRFFTChannelComponent *ADC_API::findFreqChannel(const QString &name)
{
	auto *ctrl = getFreqController();
	if(!ctrl)
		return nullptr;
	for(auto it = ctrl->m_acqNodeComponentMap.begin(); it != ctrl->m_acqNodeComponentMap.end(); ++it) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(it.value());
		if(ch && ch->name() == name)
			return ch;
	}
	return nullptr;
}

// ==================== TOOL LISTING ====================

QStringList ADC_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_adcPlugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// ==================== TIME DOMAIN - RUN CONTROL ====================

bool ADC_API::isTimeRunning()
{
	auto *ctrl = getTimeController();
	return ctrl ? ctrl->m_started : false;
}

void ADC_API::setTimeRunning(bool running)
{
	auto *ctrl = getTimeController();
	if(ctrl) {
		if(running && !ctrl->m_started) {
			Q_EMIT ctrl->requestStart();
		} else if(!running && ctrl->m_started) {
			Q_EMIT ctrl->requestStop();
		}
	}
}

void ADC_API::timeSingleShot()
{
	auto *ctrl = getTimeController();
	if(ctrl && ctrl->m_ui && ctrl->m_ui->m_singleBtn) {
		ctrl->m_ui->m_singleBtn->setChecked(true);
	}
}

// ==================== TIME DOMAIN - BUFFER/PLOT SETTINGS ====================

int ADC_API::getTimeBufferSize()
{
	auto *ctrl = getTimeController();
	return ctrl ? ctrl->m_timePlotSettingsComponent->bufferSize() : 0;
}

void ADC_API::setTimeBufferSize(int size)
{
	auto *ctrl = getTimeController();
	if(ctrl)
		ctrl->m_timePlotSettingsComponent->setBufferSize(size);
}

int ADC_API::getTimePlotSize()
{
	auto *ctrl = getTimeController();
	return ctrl ? ctrl->m_timePlotSettingsComponent->plotSize() : 0;
}

void ADC_API::setTimePlotSize(int size)
{
	auto *ctrl = getTimeController();
	if(ctrl)
		ctrl->m_timePlotSettingsComponent->setPlotSize(size);
}

bool ADC_API::isTimeSyncBufferPlot()
{
	auto *ctrl = getTimeController();
	return ctrl ? ctrl->m_timePlotSettingsComponent->syncBufferPlotSize() : false;
}

void ADC_API::setTimeSyncBufferPlot(bool sync)
{
	auto *ctrl = getTimeController();
	if(ctrl)
		ctrl->m_timePlotSettingsComponent->setSyncBufferPlotSize(sync);
}

bool ADC_API::isTimeRollingMode()
{
	auto *ctrl = getTimeController();
	return ctrl ? ctrl->m_timePlotSettingsComponent->rollingMode() : false;
}

void ADC_API::setTimeRollingMode(bool rolling)
{
	auto *ctrl = getTimeController();
	if(ctrl)
		ctrl->m_timePlotSettingsComponent->setRollingMode(rolling);
}

double ADC_API::getTimeSampleRate()
{
	auto *ctrl = getTimeController();
	return ctrl ? ctrl->m_timePlotSettingsComponent->sampleRate() : 0.0;
}

void ADC_API::setTimeSampleRate(double rate)
{
	auto *ctrl = getTimeController();
	if(ctrl)
		ctrl->m_timePlotSettingsComponent->setSampleRate(rate);
}

// ==================== TIME DOMAIN - SINGLE Y MODE ====================

bool ADC_API::isTimeSingleYMode()
{
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	return plot ? plot->singleYMode() : false;
}

void ADC_API::setTimeSingleYMode(bool single)
{
	auto *ctrl = getTimeController();
	if(!ctrl)
		return;
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(plot)
		plot->setSingleYModeAll(single);
}

// ==================== TIME DOMAIN - CHANNELS ====================

QStringList ADC_API::getTimeChannels()
{
	QStringList channels;
	auto *ctrl = getTimeController();
	if(!ctrl)
		return channels;
	for(auto it = ctrl->m_acqNodeComponentMap.begin(); it != ctrl->m_acqNodeComponentMap.end(); ++it) {
		auto *ch = dynamic_cast<GRTimeChannelComponent *>(it.value());
		if(ch)
			channels.append(ch->name());
	}
	return channels;
}

bool ADC_API::isTimeChannelEnabled(const QString &channel)
{
	auto *ch = findTimeChannel(channel);
	return ch ? ch->enabled() : false;
}

void ADC_API::setTimeChannelEnabled(const QString &channel, bool enabled)
{
	auto *ch = findTimeChannel(channel);
	if(ch && ch->ctrl()) {
		ch->ctrl()->checkBox()->setChecked(enabled);
	}
}

// ==================== TIME DOMAIN - CHANNEL Y-MODE ====================

int ADC_API::getTimeChannelYMode(const QString &channel)
{
	auto *ch = findTimeChannel(channel);
	return ch ? static_cast<int>(ch->ymode()) : 0;
}

void ADC_API::setTimeChannelYMode(const QString &channel, int mode)
{
	auto *ch = findTimeChannel(channel);
	if(ch)
		ch->setYMode(static_cast<YMode>(mode));
}

// ==================== TIME DOMAIN - CHANNEL Y-AXIS ====================

double ADC_API::getTimeChannelYMin(const QString &channel)
{
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return 0.0;
	auto *plotCh = dynamic_cast<TimePlotComponentChannel *>(ch->plotChannelCmpt());
	return plotCh && plotCh->m_timePlotYAxis ? plotCh->m_timePlotYAxis->min() : 0.0;
}

double ADC_API::getTimeChannelYMax(const QString &channel)
{
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return 0.0;
	auto *plotCh = dynamic_cast<TimePlotComponentChannel *>(ch->plotChannelCmpt());
	return plotCh && plotCh->m_timePlotYAxis ? plotCh->m_timePlotYAxis->max() : 0.0;
}

void ADC_API::setTimeChannelYMin(const QString &channel, double min)
{
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return;
	auto *plotCh = dynamic_cast<TimePlotComponentChannel *>(ch->plotChannelCmpt());
	if(plotCh && plotCh->m_timePlotYAxis)
		plotCh->m_timePlotYAxis->setMin(min);
}

void ADC_API::setTimeChannelYMax(const QString &channel, double max)
{
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return;
	auto *plotCh = dynamic_cast<TimePlotComponentChannel *>(ch->plotChannelCmpt());
	if(plotCh && plotCh->m_timePlotYAxis)
		plotCh->m_timePlotYAxis->setMax(max);
}

// ==================== TIME DOMAIN - CURSORS ====================

bool ADC_API::isTimeCursorsEnabled()
{
	auto *ctrl = getTimeController();
	if(!ctrl || !ctrl->m_plotComponentManager)
		return false;
	auto *plot = ctrl->m_plotComponentManager->plot(0);
	return plot && plot->cursor() ? plot->cursor()->isVisible() : false;
}

void ADC_API::setTimeCursorsEnabled(bool enabled)
{
	auto *ctrl = getTimeController();
	if(ctrl && ctrl->m_ui && ctrl->m_ui->m_cursor) {
		ctrl->m_ui->m_cursor->setChecked(enabled);
	}
}

// ==================== TIME DOMAIN - MEASUREMENTS ====================

QStringList ADC_API::getTimeMeasurements(const QString &channel)
{
	QStringList measurements;
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return measurements;
	auto *mgr = ch->getMeasureManager();
	if(!mgr)
		return measurements;
	auto *controller = mgr->getController();
	if(!controller)
		return measurements;
	for(const auto &info : controller->availableMeasurements()) {
		measurements.append(info.name);
	}
	return measurements;
}

void ADC_API::enableTimeMeasurement(const QString &channel, const QString &measurement)
{
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return;
	auto *mgr = ch->getMeasureManager();
	if(!mgr)
		return;
	auto *controller = mgr->getController();
	if(controller)
		controller->enableMeasurement(measurement);
}

void ADC_API::disableTimeMeasurement(const QString &channel, const QString &measurement)
{
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return;
	auto *mgr = ch->getMeasureManager();
	if(!mgr)
		return;
	auto *controller = mgr->getController();
	if(controller)
		controller->disableMeasurement(measurement);
}

double ADC_API::getTimeMeasurementValue(const QString &channel, const QString &measurement)
{
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return 0.0;
	auto *mgr = ch->getMeasureManager();
	if(!mgr)
		return 0.0;
	auto *model = mgr->getModel();
	if(!model)
		return 0.0;
	auto data = model->measurement(measurement);
	return data ? data->value() : 0.0;
}

// ==================== FREQUENCY DOMAIN - RUN CONTROL ====================

bool ADC_API::isFreqRunning()
{
	auto *ctrl = getFreqController();
	return ctrl ? ctrl->m_started : false;
}

void ADC_API::setFreqRunning(bool running)
{
	auto *ctrl = getFreqController();
	if(ctrl) {
		if(running && !ctrl->m_started) {
			Q_EMIT ctrl->requestStart();
		} else if(!running && ctrl->m_started) {
			Q_EMIT ctrl->requestStop();
		}
	}
}

void ADC_API::freqSingleShot()
{
	auto *ctrl = getFreqController();
	if(ctrl && ctrl->m_ui && ctrl->m_ui->m_singleBtn) {
		ctrl->m_ui->m_singleBtn->setChecked(true);
	}
}

// ==================== FREQUENCY DOMAIN - SETTINGS ====================

int ADC_API::getFreqBufferSize()
{
	auto *ctrl = getFreqController();
	return ctrl ? ctrl->m_fftPlotSettingsComponent->bufferSize() : 0;
}

void ADC_API::setFreqBufferSize(int size)
{
	auto *ctrl = getFreqController();
	if(ctrl)
		ctrl->m_fftPlotSettingsComponent->setBufferSize(size);
}

double ADC_API::getFreqSampleRate()
{
	auto *ctrl = getFreqController();
	return ctrl ? ctrl->m_fftPlotSettingsComponent->sampleRate() : 0.0;
}

void ADC_API::setFreqSampleRate(double rate)
{
	auto *ctrl = getFreqController();
	if(ctrl)
		ctrl->m_fftPlotSettingsComponent->setSampleRate(rate);
}

double ADC_API::getFreqOffset()
{
	auto *ctrl = getFreqController();
	return ctrl ? ctrl->m_fftPlotSettingsComponent->freqOffset() : 0.0;
}

void ADC_API::setFreqOffset(double offset)
{
	auto *ctrl = getFreqController();
	if(ctrl)
		ctrl->m_fftPlotSettingsComponent->setFreqOffset(offset);
}

// ==================== FREQUENCY DOMAIN - COMPLEX MODE ====================

bool ADC_API::isFreqComplexMode()
{
	auto *ctrl = getFreqController();
	return ctrl ? ctrl->m_fftPlotSettingsComponent->complexMode() : false;
}

void ADC_API::setFreqComplexMode(bool complex)
{
	auto *ctrl = getFreqController();
	if(ctrl && ctrl->m_ui && ctrl->m_ui->m_complex) {
		ctrl->m_ui->m_complex->setChecked(complex);
	}
}

// ==================== FREQUENCY DOMAIN - CHANNELS ====================

QStringList ADC_API::getFreqChannels()
{
	QStringList channels;
	auto *ctrl = getFreqController();
	if(!ctrl)
		return channels;
	for(auto it = ctrl->m_acqNodeComponentMap.begin(); it != ctrl->m_acqNodeComponentMap.end(); ++it) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(it.value());
		if(ch)
			channels.append(ch->name());
	}
	return channels;
}

bool ADC_API::isFreqChannelEnabled(const QString &channel)
{
	auto *ch = findFreqChannel(channel);
	return ch ? ch->enabled() : false;
}

void ADC_API::setFreqChannelEnabled(const QString &channel, bool enabled)
{
	auto *ch = findFreqChannel(channel);
	if(ch && ch->ctrl()) {
		ch->ctrl()->checkBox()->setChecked(enabled);
	}
}

// ==================== FREQUENCY DOMAIN - FFT SETTINGS ====================

int ADC_API::getFreqWindow()
{
	auto *ctrl = getFreqController();
	if(!ctrl)
		return 0;
	for(auto it = ctrl->m_acqNodeComponentMap.begin(); it != ctrl->m_acqNodeComponentMap.end(); ++it) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(it.value());
		if(ch)
			return ch->window();
	}
	return 0;
}

void ADC_API::setFreqWindow(int window)
{
	auto *ctrl = getFreqController();
	if(!ctrl)
		return;
	for(auto it = ctrl->m_acqNodeComponentMap.begin(); it != ctrl->m_acqNodeComponentMap.end(); ++it) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(it.value());
		if(ch)
			ch->setWindow(window);
	}
}

double ADC_API::getFreqPowerOffset()
{
	auto *ctrl = getFreqController();
	if(!ctrl)
		return 0.0;
	for(auto it = ctrl->m_acqNodeComponentMap.begin(); it != ctrl->m_acqNodeComponentMap.end(); ++it) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(it.value());
		if(ch)
			return ch->powerOffset();
	}
	return 0.0;
}

void ADC_API::setFreqPowerOffset(double offset)
{
	auto *ctrl = getFreqController();
	if(!ctrl)
		return;
	for(auto it = ctrl->m_acqNodeComponentMap.begin(); it != ctrl->m_acqNodeComponentMap.end(); ++it) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(it.value());
		if(ch)
			ch->setPowerOffset(offset);
	}
}

// ==================== FREQUENCY DOMAIN - CHANNEL Y-AXIS ====================

double ADC_API::getFreqChannelYMin(const QString &channel)
{
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return 0.0;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	return plotCh && plotCh->m_fftPlotYAxis ? plotCh->m_fftPlotYAxis->min() : 0.0;
}

double ADC_API::getFreqChannelYMax(const QString &channel)
{
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return 0.0;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	return plotCh && plotCh->m_fftPlotYAxis ? plotCh->m_fftPlotYAxis->max() : 0.0;
}

void ADC_API::setFreqChannelYMin(const QString &channel, double min)
{
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(plotCh && plotCh->m_fftPlotYAxis)
		plotCh->m_fftPlotYAxis->setMin(min);
}

void ADC_API::setFreqChannelYMax(const QString &channel, double max)
{
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(plotCh && plotCh->m_fftPlotYAxis)
		plotCh->m_fftPlotYAxis->setMax(max);
}

// ==================== FREQUENCY DOMAIN - MARKERS ====================

bool ADC_API::isFreqChannelMarkerEnabled(const QString &channel)
{
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return false;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	return plotCh && plotCh->markerController() ? plotCh->markerController()->enabled() : false;
}

void ADC_API::setFreqChannelMarkerEnabled(const QString &channel, bool enabled)
{
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(plotCh && plotCh->markerController())
		plotCh->markerController()->setEnabled(enabled);
}

void ADC_API::setFreqChannelMarkerCount(const QString &channel, int count)
{
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(plotCh && plotCh->markerController())
		plotCh->markerController()->setNrOfMarkers(count);
}

void ADC_API::setFreqChannelMarkerType(const QString &channel, int type)
{
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(plotCh && plotCh->markerController())
		plotCh->markerController()->setMarkerType(static_cast<PlotMarkerController::MarkerTypes>(type));
}

// ==================== FREQUENCY DOMAIN - CURSORS ====================

bool ADC_API::isFreqCursorsEnabled()
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_plotComponentManager)
		return false;
	auto *plot = ctrl->m_plotComponentManager->plot(0);
	return plot && plot->cursor() ? plot->cursor()->isVisible() : false;
}

void ADC_API::setFreqCursorsEnabled(bool enabled)
{
	auto *ctrl = getFreqController();
	if(ctrl && ctrl->m_ui && ctrl->m_ui->m_cursor) {
		ctrl->m_ui->m_cursor->setChecked(enabled);
	}
}

// ==================== XY PLOT ====================

bool ADC_API::isTimeXYPlotEnabled()
{
	auto *ctrl = getTimeController();
	if(!ctrl || !ctrl->m_plotComponentManager)
		return false;
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu())
		return false;
	return plot->plotMenu()->m_xAxisSrc->isVisible();
}

void ADC_API::setTimeXYPlotEnabled(bool enabled)
{
	auto *ctrl = getTimeController();
	if(!ctrl || !ctrl->m_plotComponentManager)
		return;
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu())
		return;
	plot->xyDockWidget()->setActivated(enabled);
	plot->plotMenu()->m_xAxisSrc->setVisible(enabled);
	plot->plotMenu()->m_xAxisShow->setVisible(enabled);
}

QString ADC_API::getTimeXYSource()
{
	auto *ctrl = getTimeController();
	if(!ctrl || !ctrl->m_plotComponentManager)
		return QString();
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot)
		return QString();
	ChannelComponent *xyCh = plot->XYXChannel();
	return xyCh ? xyCh->name() : QString();
}

void ADC_API::setTimeXYSource(const QString &channel)
{
	auto *ctrl = getTimeController();
	if(!ctrl || !ctrl->m_plotComponentManager)
		return;
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu())
		return;
	QComboBox *cb = plot->plotMenu()->m_xAxisSrc->combo();
	for(int i = 0; i < cb->count(); ++i) {
		if(cb->itemText(i) == channel) {
			cb->setCurrentIndex(i);
			return;
		}
	}
}

// ==================== AUTOSCALE ====================

bool ADC_API::isTimeAutoscaleEnabled()
{
	auto *ctrl = getTimeController();
	if(!ctrl || !ctrl->m_plotComponentManager)
		return false;
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu())
		return false;
	return plot->plotMenu()->m_autoscaleEnabled;
}

void ADC_API::setTimeAutoscaleEnabled(bool enabled)
{
	auto *ctrl = getTimeController();
	if(!ctrl || !ctrl->m_plotComponentManager)
		return;
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu())
		return;
	plot->plotMenu()->m_autoscaleBtn->onOffswitch()->setChecked(enabled);
}

bool ADC_API::isFreqAutoscaleEnabled()
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_plotComponentManager)
		return false;
	auto *plot = dynamic_cast<FFTPlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu())
		return false;
	return plot->plotMenu()->m_autoscaleEnabled;
}

void ADC_API::setFreqAutoscaleEnabled(bool enabled)
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_plotComponentManager)
		return;
	auto *plot = dynamic_cast<FFTPlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu())
		return;
	auto *settings = plot->plotMenu();
	settings->m_autoscaleEnabled = enabled;
	settings->m_yCtrl->setEnabled(!enabled);
	settings->toggleAutoScale();
}

// ==================== X-MODE ====================

int ADC_API::getTimeXMode()
{
	auto *ctrl = getTimeController();
	if(!ctrl)
		return 0;
	return ctrl->m_timePlotSettingsComponent->m_xModeCb->combo()->currentData().toInt();
}

void ADC_API::setTimeXMode(int mode)
{
	auto *ctrl = getTimeController();
	if(!ctrl)
		return;
	QComboBox *cb = ctrl->m_timePlotSettingsComponent->m_xModeCb->combo();
	for(int i = 0; i < cb->count(); ++i) {
		if(cb->itemData(i).toInt() == mode) {
			cb->setCurrentIndex(i);
			return;
		}
	}
}

int ADC_API::getFreqXMode()
{
	auto *ctrl = getFreqController();
	if(!ctrl)
		return 0;
	return ctrl->m_fftPlotSettingsComponent->m_xModeCb->combo()->currentData().toInt();
}

void ADC_API::setFreqXMode(int mode)
{
	auto *ctrl = getFreqController();
	if(!ctrl)
		return;
	QComboBox *cb = ctrl->m_fftPlotSettingsComponent->m_xModeCb->combo();
	for(int i = 0; i < cb->count(); ++i) {
		if(cb->itemData(i).toInt() == mode) {
			cb->setCurrentIndex(i);
			return;
		}
	}
}

// ==================== CHANNEL AVERAGING (FFT) ====================

bool ADC_API::isFreqChannelAveragingEnabled(const QString &channel) { return getFreqChannelAveragingSize(channel) > 1; }

void ADC_API::setFreqChannelAveragingEnabled(const QString &channel, bool enabled)
{
	if(enabled) {
		if(getFreqChannelAveragingSize(channel) <= 1) {
			setFreqChannelAveragingSize(channel, 2);
		}
	} else {
		setFreqChannelAveragingSize(channel, 1);
	}
}

int ADC_API::getFreqChannelAveragingSize(const QString &channel)
{
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return 0;
	auto *complexSigpath = dynamic_cast<GRFFTComplexChannelSigpath *>(ch->m_grtch);
	if(complexSigpath)
		return complexSigpath->averagingSize();
	auto *floatSigpath = dynamic_cast<GRFFTChannelSigpath *>(ch->m_grtch);
	if(floatSigpath)
		return floatSigpath->averagingSize();
	return 0;
}

void ADC_API::setFreqChannelAveragingSize(const QString &channel, int size)
{
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return;
	auto *fftCh = dynamic_cast<FFTChannel *>(ch->m_grtch);
	if(fftCh)
		fftCh->setAveragingSize(size);
}

// ==================== GENALYZER ====================

bool ADC_API::isGenalyzerEnabled()
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_measureComponent)
		return false;
	return ctrl->m_measureComponent->genalyzerSettings()->getConfig().enabled;
}

void ADC_API::setGenalyzerEnabled(bool enabled)
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_measureComponent)
		return;
	auto *gs = ctrl->m_measureComponent->genalyzerSettings();
	auto config = gs->getConfig();
	config.enabled = enabled;
	gs->setConfig(config);
}

int ADC_API::getGenalyzerMode()
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_measureComponent)
		return 0;
	return static_cast<int>(ctrl->m_measureComponent->genalyzerSettings()->getConfig().mode);
}

void ADC_API::setGenalyzerMode(int mode)
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_measureComponent)
		return;
	auto *gs = ctrl->m_measureComponent->genalyzerSettings();
	auto config = gs->getConfig();
	config.mode = static_cast<grutil::GenalyzerMode>(mode);
	gs->setConfig(config);
}

int ADC_API::getGenalyzerSSBWidth()
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_measureComponent)
		return 0;
	return ctrl->m_measureComponent->genalyzerSettings()->getConfig().auto_params.ssb_width;
}

void ADC_API::setGenalyzerSSBWidth(int width)
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_measureComponent)
		return;
	auto *gs = ctrl->m_measureComponent->genalyzerSettings();
	auto config = gs->getConfig();
	config.auto_params.ssb_width = width;
	gs->setConfig(config);
}

double ADC_API::getGenalyzerExpectedFreq()
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_measureComponent)
		return 0.0;
	return ctrl->m_measureComponent->genalyzerSettings()->getConfig().fixed_tone.expected_freq;
}

void ADC_API::setGenalyzerExpectedFreq(double freq)
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_measureComponent)
		return;
	auto *gs = ctrl->m_measureComponent->genalyzerSettings();
	auto config = gs->getConfig();
	config.fixed_tone.expected_freq = freq;
	gs->setConfig(config);
}

int ADC_API::getGenalyzerHarmonicOrder()
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_measureComponent)
		return 0;
	return ctrl->m_measureComponent->genalyzerSettings()->getConfig().fixed_tone.harmonic_order;
}

void ADC_API::setGenalyzerHarmonicOrder(int order)
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_measureComponent)
		return;
	auto *gs = ctrl->m_measureComponent->genalyzerSettings();
	auto config = gs->getConfig();
	config.fixed_tone.harmonic_order = order;
	gs->setConfig(config);
}

void ADC_API::setGenalyzerSSBFundamental(int ssb)
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_measureComponent)
		return;
	auto *gs = ctrl->m_measureComponent->genalyzerSettings();
	auto config = gs->getConfig();
	config.fixed_tone.ssb_fundamental = ssb;
	gs->setConfig(config);
}

void ADC_API::setGenalyzerSSBDefault(int ssb)
{
	auto *ctrl = getFreqController();
	if(!ctrl || !ctrl->m_measureComponent)
		return;
	auto *gs = ctrl->m_measureComponent->genalyzerSettings();
	auto config = gs->getConfig();
	config.fixed_tone.ssb_default = ssb;
	gs->setConfig(config);
}

double ADC_API::getGenalyzerMetric(const QString &channel, const QString &metric)
{
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return 0.0;
	auto *complexSigpath = dynamic_cast<GRFFTComplexChannelSigpath *>(ch->m_grtch);
	if(!complexSigpath)
		return 0.0;
	gn_analysis_results *results = complexSigpath->getGnAnalysis();
	if(!results || !results->rkeys || !results->rvalues)
		return 0.0;
	for(size_t i = 0; i < results->results_size; ++i) {
		if(metric == QString(results->rkeys[i]))
			return results->rvalues[i];
	}
	return 0.0;
}

void ADC_API::triggerGenalyzerAnalysis(const QString &channel)
{
	auto *ch = findFreqChannel(channel);
	if(ch)
		ch->triggerGenalyzerAnalysis();
}

#include "moc_adc_api.cpp"
