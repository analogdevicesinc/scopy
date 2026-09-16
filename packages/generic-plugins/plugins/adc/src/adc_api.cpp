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
#include "genalyzersettings.h"
#include "measurementcontroller.h"

#include <pluginbase/toolmenuentry.h>
#include <gui/plotaxis.h>
#include <gui/plotmarkercontroller.h>
#include <gui/cursorcontroller.h>
#include <gui/widgets/measurementsettings.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/toolbuttons.h>
#include <gui/plotmanager.h>
#include <QLoggingCategory>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QtGlobal>
#include <QtMath>

Q_LOGGING_CATEGORY(CAT_ADC_API, "ADC_API")

using namespace scopy;
using namespace scopy::adc;
using namespace scopy::gui;

ADC_API::ADC_API(ADCPlugin *adcPlugin)
	: ApiObject()
	, m_adcPlugin(adcPlugin)
{}

ADC_API::~ADC_API() {}

// ==================== TOOL LISTING ====================

QStringList ADC_API::getTools()
{
	clearError();
	QStringList tools;
	for(ToolMenuEntry *tool : m_adcPlugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// ==================== ERROR REPORTING ====================

QString ADC_API::lastError() const { return m_lastError; }

bool ADC_API::ok() const { return m_lastError.isEmpty(); }

void ADC_API::clearError() { m_lastError.clear(); }

bool ADC_API::fail(const QString &reason) const
{
	m_lastError = reason;
	qWarning(CAT_ADC_API) << reason;
	return false;
}

int ADC_API::failInt(const QString &reason) const
{
	fail(reason);
	return -1;
}

double ADC_API::failDouble(const QString &reason) const
{
	fail(reason);
	return qQNaN();
}

// ==================== READBACK VERIFICATION ====================

bool ADC_API::verify(const QString &what, bool actual, bool expected) const
{
	if(actual == expected)
		return true;
	return fail(QString("%1: requested %2 but reads back %3")
			    .arg(what, expected ? "true" : "false", actual ? "true" : "false"));
}

bool ADC_API::verify(const QString &what, int actual, int expected) const
{
	if(actual == expected)
		return true;
	return fail(QString("%1: requested %2 but reads back %3").arg(what).arg(expected).arg(actual));
}

bool ADC_API::verify(const QString &what, double actual, double expected) const
{
	// Spinboxes round to their display precision, so compare with a relative epsilon
	// rather than demanding an exact match.
	const double scale = qMax(1.0, qMax(qAbs(actual), qAbs(expected)));
	if(qAbs(actual - expected) <= 1e-6 * scale)
		return true;
	return fail(QString("%1: requested %2 but reads back %3").arg(what).arg(expected).arg(actual));
}

QString ADC_API::comboValues(const QComboBox *cb)
{
	QStringList values;
	for(int i = 0; i < cb->count(); ++i)
		values.append(QString("%1=%2").arg(cb->itemData(i).toInt()).arg(cb->itemText(i)));
	return values.join(", ");
}

// ==================== LOOKUPS ====================

ADCTimeInstrumentController *ADC_API::getTimeController()
{
	for(auto *ctrl : m_adcPlugin->m_ctrls) {
		auto *timeCtrl = dynamic_cast<ADCTimeInstrumentController *>(ctrl);
		if(timeCtrl)
			return timeCtrl;
	}
	fail("no ADC - Time instrument found; is the device connected?");
	return nullptr;
}

TimePlotComponentSettings *ADC_API::getTimePlotSettings()
{
	auto *ctrl = getTimeController();
	if(!ctrl)
		return nullptr;
	if(!ctrl->m_plotComponentManager) {
		fail("time instrument has no plot manager");
		return nullptr;
	}
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu()) {
		fail("time instrument has no plot settings menu");
		return nullptr;
	}
	return plot->plotMenu();
}

ADCFFTInstrumentController *ADC_API::getFreqController()
{
	for(auto *ctrl : m_adcPlugin->m_ctrls) {
		auto *fftCtrl = dynamic_cast<ADCFFTInstrumentController *>(ctrl);
		if(fftCtrl)
			return fftCtrl;
	}
	fail("no ADC - Frequency instrument found; is the device connected?");
	return nullptr;
}

GRTimeChannelComponent *ADC_API::findTimeChannel(const QString &name)
{
	auto *ctrl = getTimeController();
	if(!ctrl)
		return nullptr;
	QStringList known;
	for(auto *c : ctrl->components()) {
		auto *ch = dynamic_cast<GRTimeChannelComponent *>(c);
		if(!ch)
			continue;
		if(ch->name() == name)
			return ch;
		known.append(ch->name());
	}
	fail(QString("no such time channel '%1'; available: %2").arg(name, known.join(", ")));
	return nullptr;
}

QStringList ADC_API::freqChannelNames()
{
	QStringList names;
	auto *ctrl = getFreqController();
	if(!ctrl)
		return names;
	for(auto *c : ctrl->components()) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(c);
		if(ch)
			names.append(ch->name());
	}
	return names;
}

GRFFTChannelComponent *ADC_API::findFreqChannel(const QString &name)
{
	auto *ctrl = getFreqController();
	if(!ctrl)
		return nullptr;
	// Search m_components rather than m_acqNodeComponentMap: complex channels are
	// never inserted into that map, so a map-based lookup cannot see them and every
	// setter targeting e.g. "voltage0-voltage1" would silently no-op.
	for(auto *c : ctrl->components()) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(c);
		if(ch && ch->name() == name)
			return ch;
	}
	fail(QString("no such freq channel '%1'; available: %2").arg(name, freqChannelNames().join(", ")));
	return nullptr;
}

GRFFTChannelComponent *ADC_API::findComplexChannel(const QString &name)
{
	auto *ch = findFreqChannel(name);
	if(!ch)
		return nullptr;
	if(!ch->isComplex()) {
		fail(QString("channel '%1' is not complex; genalyzer requires a complex channel").arg(name));
		return nullptr;
	}
	return ch;
}

// ==================== TIME DOMAIN - RUN CONTROL ====================

bool ADC_API::isTimeRunning()
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	return ctrl->m_started;
}

bool ADC_API::setTimeRunning(bool running)
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	// requestStart/requestStop are direct connections down to onStart()/onStop(), so
	// m_started is already updated when the emit returns and can be verified here.
	if(running && !ctrl->m_started) {
		Q_EMIT ctrl->requestStart();
	} else if(!running && ctrl->m_started) {
		Q_EMIT ctrl->requestStop();
	}
	return verify("time run state", ctrl->m_started, running);
}

bool ADC_API::timeSingleShot()
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	if(!ctrl->m_ui || !ctrl->m_ui->m_singleBtn)
		return fail("time instrument has no single shot button");
	ctrl->m_ui->m_singleBtn->setChecked(true);
	return verify("time single shot", ctrl->m_ui->m_singleBtn->isChecked(), true);
}

// ==================== TIME DOMAIN - BUFFER/PLOT SETTINGS ====================

int ADC_API::getTimeBufferSize()
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return -1;
	return (int)ctrl->m_timePlotSettingsComponent->bufferSize();
}

bool ADC_API::setTimeBufferSize(int size)
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	auto *settings = ctrl->m_timePlotSettingsComponent;
	gui::MenuSpinbox *spin = settings->m_bufferSizeSpin;
	spin->setValue(size);
	// The spinbox clamps to its own range, so an out-of-range request shows up here.
	return verify(QString("buffer size (allowed %1..%2)").arg(spin->min()).arg(spin->max()),
		      (int)settings->bufferSize(), size);
}

int ADC_API::getTimePlotSize()
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return -1;
	return (int)ctrl->m_timePlotSettingsComponent->plotSize();
}

bool ADC_API::setTimePlotSize(int size)
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	auto *settings = ctrl->m_timePlotSettingsComponent;
	gui::MenuSpinbox *spin = settings->m_plotSizeSpin;
	if(!spin->isEnabled())
		return fail("plot size spinbox is disabled; disable SYNC BUFFER-PLOT SIZES first");
	spin->setValue(size);
	// Plot size cannot go below the buffer size - the spinbox min tracks it.
	return verify(QString("plot size (allowed %1..%2)").arg(spin->min()).arg(spin->max()),
		      (int)settings->plotSize(), size);
}

bool ADC_API::isTimeSyncBufferPlot()
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	return ctrl->m_timePlotSettingsComponent->m_syncBufferPlot->onOffswitch()->isChecked();
}

bool ADC_API::setTimeSyncBufferPlot(bool sync)
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	auto *settings = ctrl->m_timePlotSettingsComponent;
	settings->m_syncBufferPlot->onOffswitch()->setChecked(sync);
	if(!verify("sync buffer-plot switch", settings->m_syncBufferPlot->onOffswitch()->isChecked(), sync))
		return false;
	// The switch only observable effect is locking the plot size / rolling mode widgets;
	// syncBufferPlotSize() is never updated by the GUI so it cannot be used as readback.
	return verify("sync buffer-plot effect on plot size spinbox", settings->m_plotSizeSpin->isEnabled(), !sync);
}

bool ADC_API::isTimeRollingMode()
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	return ctrl->m_timePlotSettingsComponent->rollingMode();
}

bool ADC_API::setTimeRollingMode(bool rolling)
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	auto *settings = ctrl->m_timePlotSettingsComponent;
	if(!settings->m_rollingModeSw->onOffswitch()->isEnabled())
		return fail("rolling mode switch is disabled; disable SYNC BUFFER-PLOT SIZES first");
	settings->m_rollingModeSw->onOffswitch()->setChecked(rolling);
	return verify("rolling mode", settings->rollingMode(), rolling);
}

double ADC_API::getTimeSampleRate()
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return qQNaN();
	return ctrl->m_timePlotSettingsComponent->sampleRate();
}

bool ADC_API::setTimeSampleRate(double rate)
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	auto *settings = ctrl->m_timePlotSettingsComponent;
	gui::MenuSpinbox *spin = settings->m_sampleRateSpin;
	if(!spin->isEnabled())
		return fail("sample rate spinbox is disabled; it is only writable in the 'Time - override "
			    "samplerate' XMode");
	spin->setValue(rate);
	return verify(QString("sample rate (allowed %1..%2)").arg(spin->min()).arg(spin->max()),
		      settings->sampleRate(), rate);
}

// ==================== TIME DOMAIN - SINGLE Y MODE ====================

bool ADC_API::isTimeSingleYMode()
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager)
		return fail("time instrument has no plot manager");
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot)
		return fail("time instrument has no plot");
	return plot->singleYMode();
}

bool ADC_API::setTimeSingleYMode(bool single)
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager)
		return fail("time instrument has no plot manager");
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot)
		return fail("time instrument has no plot");
	// No menu widget drives this; the per-channel Y-AXIS collapse headers do it one
	// channel at a time, so call the plot-wide setter directly.
	plot->setSingleYModeAll(single);
	return verify("single Y mode", plot->singleYMode(), single);
}

// ==================== TIME DOMAIN - CHANNELS ====================

QStringList ADC_API::getTimeChannels()
{
	clearError();
	QStringList channels;
	auto *ctrl = getTimeController();
	if(!ctrl)
		return channels;
	for(auto *c : ctrl->components()) {
		auto *ch = dynamic_cast<GRTimeChannelComponent *>(c);
		if(ch)
			channels.append(ch->name());
	}
	return channels;
}

bool ADC_API::isTimeChannelEnabled(const QString &channel)
{
	clearError();
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return false;
	return ch->enabled();
}

bool ADC_API::setTimeChannelEnabled(const QString &channel, bool enabled)
{
	clearError();
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return false;
	if(!ch->ctrl() || !ch->ctrl()->checkBox())
		return fail(QString("channel '%1' has no enable checkbox").arg(channel));
	ch->ctrl()->checkBox()->setChecked(enabled);
	return verify(QString("channel '%1' enabled").arg(channel), ch->enabled(), enabled);
}

// ==================== TIME DOMAIN - CHANNEL Y-MODE ====================

int ADC_API::getTimeChannelYMode(const QString &channel)
{
	clearError();
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return -1;
	return static_cast<int>(ch->ymode());
}

bool ADC_API::setTimeChannelYMode(const QString &channel, int mode)
{
	clearError();
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return false;
	if(mode < YMODE_COUNT || mode > YMODE_SCALE_OVERRIDE)
		return fail(QString("invalid Y mode %1; expected 0=COUNT, 1=FS, 2=SCALE, 3=SCALE_OVERRIDE").arg(mode));

	QComboBox *cb = ch->m_ymodeCb->combo();
	// Combo index and YMode value diverge, and YMODE_SCALE is only present on channels
	// that expose a scale attribute - findData() rejects it on the others for us.
	int idx = cb->findData(mode);
	if(idx < 0) {
		QStringList available;
		for(int i = 0; i < cb->count(); ++i)
			available.append(QString::number(cb->itemData(i).toInt()));
		return fail(QString("Y mode %1 is not available on channel '%2'; available: %3")
				    .arg(mode)
				    .arg(channel, available.join(", ")));
	}
	cb->setCurrentIndex(idx);
	// Verify the combo, not just ch->ymode(): the latter is written by this same setter's
	// signal handler, so checking it alone cannot distinguish "applied" from "stored".
	if(cb->currentData().toInt() != mode)
		return fail(QString("channel '%1' Y mode did not apply").arg(channel));
	return verify(QString("channel '%1' Y mode").arg(channel), static_cast<int>(ch->ymode()), mode);
}

// ==================== TIME DOMAIN - CHANNEL Y-AXIS ====================

double ADC_API::getTimeChannelYMin(const QString &channel)
{
	clearError();
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return qQNaN();
	auto *plotCh = dynamic_cast<TimePlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->m_timePlotYAxis)
		return failDouble(QString("channel '%1' has no time plot Y axis").arg(channel));
	return plotCh->m_timePlotYAxis->min();
}

double ADC_API::getTimeChannelYMax(const QString &channel)
{
	clearError();
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return qQNaN();
	auto *plotCh = dynamic_cast<TimePlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->m_timePlotYAxis)
		return failDouble(QString("channel '%1' has no time plot Y axis").arg(channel));
	return plotCh->m_timePlotYAxis->max();
}

bool ADC_API::setTimeChannelYMin(const QString &channel, double min)
{
	clearError();
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return false;
	auto *plotCh = dynamic_cast<TimePlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->m_timePlotYAxis)
		return fail(QString("channel '%1' has no time plot Y axis").arg(channel));
	plotCh->m_timePlotYAxis->setMin(min);
	// The axis takes any value while the menu spinbox clamps to +-1e9; checking both
	// catches a request that would leave the plot and the menu disagreeing.
	return verify(QString("channel '%1' Y min").arg(channel), plotCh->m_timePlotYAxis->min(), min) &&
		verify(QString("channel '%1' Y min (menu spinbox)").arg(channel), ch->yMin(), min);
}

bool ADC_API::setTimeChannelYMax(const QString &channel, double max)
{
	clearError();
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return false;
	auto *plotCh = dynamic_cast<TimePlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->m_timePlotYAxis)
		return fail(QString("channel '%1' has no time plot Y axis").arg(channel));
	plotCh->m_timePlotYAxis->setMax(max);
	return verify(QString("channel '%1' Y max").arg(channel), plotCh->m_timePlotYAxis->max(), max) &&
		verify(QString("channel '%1' Y max (menu spinbox)").arg(channel), ch->yMax(), max);
}

// ==================== TIME DOMAIN - CURSORS ====================

bool ADC_API::isTimeCursorsEnabled()
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager)
		return fail("time instrument has no plot manager");
	auto *plot = ctrl->m_plotComponentManager->plot(0);
	if(!plot || !plot->cursor())
		return fail("time instrument has no cursor controller");
	return plot->cursor()->isVisible();
}

bool ADC_API::setTimeCursorsEnabled(bool enabled)
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	if(!ctrl->m_ui || !ctrl->m_ui->m_cursor)
		return fail("time instrument has no cursor button");
	if(!ctrl->m_plotComponentManager)
		return fail("time instrument has no plot manager");
	auto *plot = ctrl->m_plotComponentManager->plot(0);
	if(!plot || !plot->cursor())
		return fail("time instrument has no cursor controller");
	// Drive the toolbar button so the cursor settings hover menu follows along.
	ctrl->m_ui->m_cursor->setChecked(enabled);
	return verify("cursors enabled", plot->cursor()->isVisible(), enabled);
}

// ==================== TIME DOMAIN - MEASUREMENTS ====================

QStringList ADC_API::getTimeMeasurements(const QString &channel)
{
	clearError();
	QStringList measurements;
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return measurements;
	auto *mgr = ch->getMeasureManager();
	if(!mgr) {
		fail(QString("channel '%1' has no measure manager").arg(channel));
		return measurements;
	}
	auto *controller = mgr->getController();
	if(!controller) {
		fail(QString("channel '%1' has no measurement controller").arg(channel));
		return measurements;
	}
	for(const auto &info : controller->availableMeasurements()) {
		measurements.append(info.name);
	}
	return measurements;
}

bool ADC_API::enableTimeMeasurement(const QString &channel, const QString &measurement)
{
	clearError();
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return false;
	auto *mgr = ch->getMeasureManager();
	if(!mgr)
		return fail(QString("channel '%1' has no measure manager").arg(channel));
	auto *controller = mgr->getController();
	if(!controller)
		return fail(QString("channel '%1' has no measurement controller").arg(channel));

	QStringList known;
	for(const auto &info : controller->availableMeasurements()) {
		known.append(info.name);
	}
	if(!known.contains(measurement))
		return fail(QString("no such measurement '%1' on channel '%2'; available: %3")
				    .arg(measurement, channel, known.join(", ")));

	// enableMeasurement() refuses to add a second label, so an already enabled
	// measurement is a no-op that already satisfies the request.
	if(controller->findMeasurementLabel(measurement))
		return true;
	if(!controller->enableMeasurement(measurement))
		return fail(QString("could not enable measurement '%1' on channel '%2'").arg(measurement, channel));
	return verify(QString("measurement '%1' on channel '%2' enabled").arg(measurement, channel),
		      controller->findMeasurementLabel(measurement) != nullptr, true);
}

bool ADC_API::disableTimeMeasurement(const QString &channel, const QString &measurement)
{
	clearError();
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return false;
	auto *mgr = ch->getMeasureManager();
	if(!mgr)
		return fail(QString("channel '%1' has no measure manager").arg(channel));
	auto *controller = mgr->getController();
	if(!controller)
		return fail(QString("channel '%1' has no measurement controller").arg(channel));

	// disableMeasurement() dereferences the model entry unconditionally, so reject
	// unknown names before calling it.
	QStringList known;
	for(const auto &info : controller->availableMeasurements()) {
		known.append(info.name);
	}
	if(!known.contains(measurement))
		return fail(QString("no such measurement '%1' on channel '%2'; available: %3")
				    .arg(measurement, channel, known.join(", ")));

	if(!controller->findMeasurementLabel(measurement))
		return true;
	controller->disableMeasurement(measurement);
	return verify(QString("measurement '%1' on channel '%2' disabled").arg(measurement, channel),
		      controller->findMeasurementLabel(measurement) != nullptr, false);
}

double ADC_API::getTimeMeasurementValue(const QString &channel, const QString &measurement)
{
	clearError();
	auto *ch = findTimeChannel(channel);
	if(!ch)
		return qQNaN();
	auto *mgr = ch->getMeasureManager();
	if(!mgr)
		return failDouble(QString("channel '%1' has no measure manager").arg(channel));
	auto *model = mgr->getModel();
	if(!model)
		return failDouble(QString("channel '%1' has no measure model").arg(channel));
	auto data = model->measurement(measurement);
	if(!data)
		return failDouble(QString("no such measurement '%1' on channel '%2'").arg(measurement, channel));
	// Every acquisition clears the measured flag before recomputing, so an unmeasured
	// entry means there is no fresh value to report rather than a value of 0.
	if(!data->measured())
		return failDouble(QString("measurement '%1' on channel '%2' has no value yet; is the instrument "
					  "running?")
					  .arg(measurement, channel));
	return data->value();
}

// ==================== TIME DOMAIN - AUTOSCALE ====================

bool ADC_API::isTimeAutoscaleEnabled()
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager)
		return fail("time instrument has no plot manager");
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu())
		return fail("time instrument has no plot settings menu");
	return plot->plotMenu()->m_autoscaleEnabled;
}

bool ADC_API::setTimeAutoscaleEnabled(bool enabled)
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager)
		return fail("time instrument has no plot manager");
	auto *plot = dynamic_cast<TimePlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu())
		return fail("time instrument has no plot settings menu");
	auto *menu = plot->plotMenu();
	menu->m_autoscaleBtn->onOffswitch()->setChecked(enabled);
	return verify("autoscale enabled", menu->m_autoscaleEnabled, enabled);
}

// ==================== TIME DOMAIN - X-MODE ====================

int ADC_API::getTimeXMode()
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return -1;
	return static_cast<int>(ctrl->m_timePlotSettingsComponent->xMode());
}

bool ADC_API::setTimeXMode(int mode)
{
	clearError();
	auto *ctrl = getTimeController();
	if(!ctrl)
		return false;
	auto *settings = ctrl->m_timePlotSettingsComponent;
	// XMODE_TIME is only inserted once a channel reports a sample rate, so the set of
	// valid values is runtime dependent - setXMode() rejects the absent ones for us.
	if(!settings->setXMode(static_cast<TimePlotManagerSettings::XMode>(mode)))
		return fail(QString("invalid X mode %1; available: %2")
				    .arg(mode)
				    .arg(comboValues(settings->m_xModeCb->combo())));
	return verify("X mode", static_cast<int>(settings->xMode()), mode);
}

// ============================================================================
// FREQUENCY DOMAIN
//
// Every setter drives the same widget a user would operate, then reads the
// resulting state back off the target (signal path, plot axis, controller) and
// reports failure when it did not stick. Genalyzer lives in its own section.
// ============================================================================

// ==================== FREQUENCY DOMAIN - RUN CONTROL ====================

bool ADC_API::isFreqRunning()
{
	clearError();
	auto *ctrl = getFreqController();
	return ctrl ? ctrl->m_started : false;
}

bool ADC_API::setFreqRunning(bool running)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_ui || !ctrl->m_ui->m_runBtn)
		return fail("frequency instrument has no run button");
	if(ctrl->m_started == running)
		return true; // already in the requested state

	// The run button chain (toggled -> requestStart -> start -> arm -> onStart) is
	// made of direct connections, so m_started is up to date once we return.
	ctrl->m_ui->m_runBtn->setChecked(running);
	return verify("freq running", ctrl->m_started, running);
}

bool ADC_API::freqSingleShot()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_ui || !ctrl->m_ui->m_singleBtn)
		return fail("frequency instrument has no single shot button");

	ctrl->m_ui->m_singleBtn->setChecked(true);
	// A single shot may already have completed by the time we get here, in which case
	// the button was reset and the controller stopped again. Only the case where
	// neither happened means the click was swallowed.
	if(!ctrl->m_ui->m_singleBtn->isChecked() && !ctrl->m_started)
		return fail("freq single shot: acquisition did not start");
	return true;
}

// ==================== FREQUENCY DOMAIN - SETTINGS ====================

int ADC_API::getFreqBufferSize()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return -1;
	return (int)ctrl->m_fftPlotSettingsComponent->bufferSize();
}

bool ADC_API::setFreqBufferSize(int size)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	auto *settings = ctrl->m_fftPlotSettingsComponent;
	gui::MenuSpinbox *spin = settings->m_bufferSizeSpin;
	if(!spin)
		return fail("frequency instrument has no FFT size spinbox");
	if(size < spin->min() || size > spin->max())
		return fail(QString("freq buffer size %1 is out of range [%2, %3]")
				    .arg(size)
				    .arg((int)spin->min())
				    .arg((int)spin->max()));

	// Drive the spinbox rather than setBufferSize() so the menu shows the new value.
	spin->setValue(size);
	// setBufferSize() early-returns when unchanged, so an already correct value is a
	// success, not a no-op failure - the readback covers both cases.
	return verify("freq buffer size", (int)settings->bufferSize(), size);
}

double ADC_API::getFreqSampleRate()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return qQNaN();
	return ctrl->m_fftPlotSettingsComponent->sampleRate();
}

bool ADC_API::setFreqSampleRate(double rate)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	auto *settings = ctrl->m_fftPlotSettingsComponent;
	gui::MenuSpinbox *spin = settings->m_sampleRateSpin;
	if(!spin)
		return fail("frequency instrument has no sample rate spinbox");
	if(!spin->isEnabled())
		return fail("sample rate spinbox is disabled; the device reports its own sample rate in this "
			    "X-mode");
	if(rate < spin->min() || rate > spin->max())
		return fail(QString("freq sample rate %1 is out of range [%2, %3]")
				    .arg(rate)
				    .arg(spin->min())
				    .arg(spin->max()));

	spin->setValue(rate);
	return verify("freq sample rate", settings->sampleRate(), rate);
}

double ADC_API::getFreqOffset()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return qQNaN();
	return ctrl->m_fftPlotSettingsComponent->freqOffset();
}

bool ADC_API::setFreqOffset(double offset)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	auto *settings = ctrl->m_fftPlotSettingsComponent;
	gui::MenuSpinbox *spin = settings->m_freqOffsetSpin;
	if(!spin)
		return fail("frequency instrument has no frequency offset spinbox");
	// The spinbox starts out disabled and is only enabled by the X-modes that make an
	// offset meaningful; writing to it otherwise would be invisible to the user.
	if(!spin->isEnabled())
		return fail("frequency offset spinbox is disabled; select an X-mode that supports a frequency "
			    "offset first");
	if(offset < spin->min() || offset > spin->max())
		return fail(QString("freq offset %1 is out of range [%2, %3]")
				    .arg(offset)
				    .arg(spin->min())
				    .arg(spin->max()));

	spin->setValue(offset);
	return verify("freq offset", settings->freqOffset(), offset);
}

// ==================== FREQUENCY DOMAIN - COMPLEX MODE ====================

bool ADC_API::isFreqComplexMode()
{
	clearError();
	auto *ctrl = getFreqController();
	return ctrl ? ctrl->m_fftPlotSettingsComponent->complexMode() : false;
}

bool ADC_API::setFreqComplexMode(bool complex)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_ui || !ctrl->m_ui->m_complex)
		return fail("frequency instrument has no complex mode button");

	ctrl->m_ui->m_complex->setChecked(complex);
	return verify("freq complex mode", ctrl->m_fftPlotSettingsComponent->complexMode(), complex);
}

// ==================== FREQUENCY DOMAIN - CHANNELS ====================

QStringList ADC_API::getFreqChannels()
{
	clearError();
	// Lists real and complex channels alike: every name returned here must be a name
	// findFreqChannel() accepts, otherwise the setters would reject their own channels.
	return freqChannelNames();
}

QStringList ADC_API::getComplexChannels()
{
	clearError();
	QStringList channels;
	auto *ctrl = getFreqController();
	if(!ctrl)
		return channels;
	for(auto *c : ctrl->components()) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(c);
		if(ch && ch->isComplex())
			channels.append(ch->name());
	}
	return channels;
}

bool ADC_API::isFreqChannelEnabled(const QString &channel)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	return ch ? ch->enabled() : false;
}

bool ADC_API::setFreqChannelEnabled(const QString &channel, bool enabled)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return false;
	if(!ch->ctrl() || !ch->ctrl()->checkBox())
		return fail(QString("freq channel '%1' has no enable checkbox").arg(channel));

	ch->ctrl()->checkBox()->setChecked(enabled);
	if(enabled && ch->enabled() != enabled) {
		// GRFFTChannelComponent::enabled() also requires the channel kind to match the
		// instrument mode, so a complex channel stays off while in real mode.
		auto *ctrl = getFreqController();
		bool complexMode = ctrl && ctrl->m_fftPlotSettingsComponent->complexMode();
		if(ch->isComplex() != complexMode)
			return fail(QString("freq channel '%1' is %2 but the instrument is in %3 mode; call "
					    "setFreqComplexMode(%4) first")
					    .arg(channel, ch->isComplex() ? "complex" : "real",
						 complexMode ? "complex" : "real",
						 ch->isComplex() ? "true" : "false"));
	}
	return verify(QString("freq channel '%1' enabled").arg(channel), ch->enabled(), enabled);
}

// ==================== FREQUENCY DOMAIN - FFT SETTINGS ====================

int ADC_API::getFreqWindow()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return -1;
	for(auto *c : ctrl->components()) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(c);
		if(ch)
			return ch->window(); // the window is a plot-wide setting; report the first channel's
	}
	return failInt("no freq channels available");
}

bool ADC_API::setFreqWindow(int window)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager)
		return fail("frequency instrument has no plots");

	QStringList valid;
	bool applied = false;
	for(auto *p : ctrl->m_plotComponentManager->plots()) {
		auto *plot = dynamic_cast<FFTPlotComponent *>(p);
		if(!plot || !plot->plotMenu() || !plot->plotMenu()->m_windowCb)
			continue;
		QComboBox *cb = plot->plotMenu()->m_windowCb->combo();
		int idx = cb->findData(window);
		if(idx < 0) {
			valid.clear();
			for(int i = 0; i < cb->count(); ++i)
				valid.append(QString("%1=%2").arg(cb->itemData(i).toInt()).arg(cb->itemText(i)));
			continue;
		}
		// Drive the combo so its per-channel wiring pushes the window to every channel.
		cb->setCurrentIndex(idx);
		applied = true;
	}
	if(!applied)
		return fail(QString("invalid freq window %1; valid values: %2").arg(window).arg(valid.join(", ")));

	QStringList rejected;
	int channels = 0;
	for(auto *c : ctrl->components()) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(c);
		if(!ch)
			continue;
		channels++;
		if(ch->window() != window)
			rejected.append(ch->name());
	}
	if(channels == 0)
		return fail("no freq channels available");
	if(!rejected.isEmpty())
		return fail(QString("freq window %1 was not applied to: %2").arg(window).arg(rejected.join(", ")));
	return true;
}

double ADC_API::getFreqPowerOffset()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return qQNaN();
	for(auto *c : ctrl->components()) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(c);
		if(ch)
			return ch->powerOffset(); // plot-wide setting; report the first channel's
	}
	return failDouble("no freq channels available");
}

bool ADC_API::setFreqPowerOffset(double offset)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager)
		return fail("frequency instrument has no plots");

	bool applied = false;
	for(auto *p : ctrl->m_plotComponentManager->plots()) {
		auto *plot = dynamic_cast<FFTPlotComponent *>(p);
		if(!plot || !plot->plotMenu() || !plot->plotMenu()->m_yPwrOffset)
			continue;
		gui::MenuSpinbox *spin = plot->plotMenu()->m_yPwrOffset;
		if(offset < spin->min() || offset > spin->max())
			return fail(QString("freq power offset %1 is out of range [%2, %3]")
					    .arg(offset)
					    .arg(spin->min())
					    .arg(spin->max()));
		spin->setValue(offset);
		applied = true;
	}
	if(!applied)
		return fail("frequency instrument has no power offset spinbox");

	QStringList rejected;
	int channels = 0;
	for(auto *c : ctrl->components()) {
		auto *ch = dynamic_cast<GRFFTChannelComponent *>(c);
		if(!ch)
			continue;
		channels++;
		if(qAbs(ch->powerOffset() - offset) > 1e-6 * qMax(1.0, qAbs(offset)))
			rejected.append(ch->name());
	}
	if(channels == 0)
		return fail("no freq channels available");
	if(!rejected.isEmpty())
		return fail(QString("freq power offset %1 was not applied to: %2")
				    .arg(offset)
				    .arg(rejected.join(", ")));
	return true;
}

// ==================== FREQUENCY DOMAIN - CHANNEL Y-AXIS ====================

double ADC_API::getFreqChannelYMin(const QString &channel)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return qQNaN();
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->m_fftPlotYAxis)
		return failDouble(QString("freq channel '%1' has no Y axis").arg(channel));
	return plotCh->m_fftPlotYAxis->min();
}

double ADC_API::getFreqChannelYMax(const QString &channel)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return qQNaN();
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->m_fftPlotYAxis)
		return failDouble(QString("freq channel '%1' has no Y axis").arg(channel));
	return plotCh->m_fftPlotYAxis->max();
}

bool ADC_API::setFreqChannelYMin(const QString &channel, double min)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return false;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->m_fftPlotYAxis)
		return fail(QString("freq channel '%1' has no Y axis").arg(channel));
	if(!ch->m_yCtrl || !ch->m_yCtrl->minSpinbox())
		return fail(QString("freq channel '%1' has no Y min spinbox").arg(channel));

	gui::MenuSpinbox *spin = ch->m_yCtrl->minSpinbox();
	if(min < spin->min() || min > spin->max())
		return fail(QString("freq Y min %1 is out of range [%2, %3]")
				    .arg(min)
				    .arg(spin->min())
				    .arg(spin->max()));

	spin->setValue(min);
	return verify(QString("freq channel '%1' Y min").arg(channel), plotCh->m_fftPlotYAxis->min(), min);
}

bool ADC_API::setFreqChannelYMax(const QString &channel, double max)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return false;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->m_fftPlotYAxis)
		return fail(QString("freq channel '%1' has no Y axis").arg(channel));
	if(!ch->m_yCtrl || !ch->m_yCtrl->maxSpinbox())
		return fail(QString("freq channel '%1' has no Y max spinbox").arg(channel));

	gui::MenuSpinbox *spin = ch->m_yCtrl->maxSpinbox();
	if(max < spin->min() || max > spin->max())
		return fail(QString("freq Y max %1 is out of range [%2, %3]")
				    .arg(max)
				    .arg(spin->min())
				    .arg(spin->max()));

	spin->setValue(max);
	return verify(QString("freq channel '%1' Y max").arg(channel), plotCh->m_fftPlotYAxis->max(), max);
}

// ==================== FREQUENCY DOMAIN - MARKERS ====================

bool ADC_API::isFreqChannelMarkerEnabled(const QString &channel)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return false;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->markerController()) {
		fail(QString("freq channel '%1' has no marker controller").arg(channel));
		return false;
	}
	auto *mc = plotCh->markerController();
	// Markers are drawn only when the controller is enabled (channel on plot) *and* a
	// marker type is selected - the menu section toggle drives the latter.
	return mc->enabled() && mc->markerType() != PlotMarkerController::MC_NONE;
}

bool ADC_API::setFreqChannelMarkerEnabled(const QString &channel, bool enabled)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return false;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->markerController())
		return fail(QString("freq channel '%1' has no marker controller").arg(channel));
	auto *mc = plotCh->markerController();
	if(!ch->m_markerSection)
		return fail(QString("freq channel '%1' has no marker menu").arg(channel));

	// The MARKER section header is what selects/clears the marker type.
	ch->applyMarkerEnabled(enabled);
	if(ch->m_markerSection->collapseSection()->header()->isChecked() != enabled)
		return fail(QString("freq channel '%1' markers: the MARKER section did not toggle").arg(channel));
	if(enabled && !mc->enabled())
		return fail(QString("marker section enabled but freq channel '%1' is disabled, so no markers are "
				    "drawn; enable the channel first")
				    .arg(channel));
	return verify(QString("freq channel '%1' markers").arg(channel),
		      mc->markerType() != PlotMarkerController::MC_NONE, enabled);
}

int ADC_API::getFreqChannelMarkerCount(const QString &channel)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return -1;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->markerController())
		return failInt(QString("freq channel '%1' has no marker controller").arg(channel));
	return plotCh->markerController()->nrOfMarkers();
}

bool ADC_API::setFreqChannelMarkerCount(const QString &channel, int count)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return false;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->markerController())
		return fail(QString("freq channel '%1' has no marker controller").arg(channel));
	auto *mc = plotCh->markerController();
	if(!ch->m_markerCntSpin)
		return fail(QString("freq channel '%1' has no marker count spinbox").arg(channel));

	// Read the range off the widget: the combo lambda raises the minimum to 2 for single
	// tone markers, so the permitted range depends on the current marker type.
	gui::MenuSpinbox *spin = ch->m_markerCntSpin;
	if(count < spin->min() || count > spin->max())
		return fail(QString("freq marker count %1 is out of range [%2, %3] for the current marker type (%4)")
				    .arg(count)
				    .arg((int)spin->min())
				    .arg((int)spin->max())
				    .arg((int)mc->markerType()));
	// Image markers are always a fixed triplet, so a count would be ignored.
	if(mc->markerType() == PlotMarkerController::MC_IMAGE)
		return fail(QString("freq channel '%1' uses image markers, which are always 3; change the marker "
				    "type before setting a count")
				    .arg(channel));

	ch->applyMarkerCount(count);
	return verify(QString("freq channel '%1' marker count").arg(channel), mc->nrOfMarkers(), count);
}

int ADC_API::getFreqChannelMarkerType(const QString &channel)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return -1;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->markerController())
		return failInt(QString("freq channel '%1' has no marker controller").arg(channel));
	return (int)plotCh->markerController()->markerType();
}

bool ADC_API::setFreqChannelMarkerType(const QString &channel, int type)
{
	clearError();
	if(type < PlotMarkerController::MC_NONE || type > PlotMarkerController::MC_IMAGE)
		return fail(QString("invalid freq marker type %1; expected %2=none, %3=peak, %4=fixed, "
				    "%5=single tone, %6=image")
				    .arg(type)
				    .arg((int)PlotMarkerController::MC_NONE)
				    .arg((int)PlotMarkerController::MC_PEAK)
				    .arg((int)PlotMarkerController::MC_FIXED)
				    .arg((int)PlotMarkerController::MC_SINGLETONE)
				    .arg((int)PlotMarkerController::MC_IMAGE));

	auto *ch = findFreqChannel(channel);
	if(!ch)
		return false;
	auto *plotCh = dynamic_cast<FFTPlotComponentChannel *>(ch->plotChannelCmpt());
	if(!plotCh || !plotCh->markerController())
		return fail(QString("freq channel '%1' has no marker controller").arg(channel));
	auto *mc = plotCh->markerController();
	if(!ch->m_markerCombo || !ch->m_markerSection)
		return fail(QString("freq channel '%1' has no marker menu").arg(channel));

	if(type == PlotMarkerController::MC_NONE) {
		// MC_NONE is not a combo entry; it is what turning the section off selects.
		ch->applyMarkerEnabled(false);
		return verify(QString("freq channel '%1' marker type").arg(channel), (int)mc->markerType(), type);
	}

	// While the MARKER section is off the header lambda has forced the controller to
	// MC_NONE and re-derives the type from the combo on every check/uncheck cycle, so a
	// type written now would not be the type in effect later.
	if(!ch->m_markerSection->collapseSection()->header()->isChecked())
		return fail(QString("markers are disabled on '%1'; call setFreqChannelMarkerEnabled('%1', true) "
				    "first")
				    .arg(channel));

	// applyMarkerType() silently does nothing when the combo has no such entry, so check
	// first rather than letting it no-op.
	QComboBox *cb = ch->m_markerCombo->combo();
	if(cb->findData(type) < 0) {
		QStringList valid;
		for(int i = 0; i < cb->count(); ++i)
			valid.append(QString("%1=%2").arg(cb->itemData(i).toInt()).arg(cb->itemText(i)));
		return fail(QString("freq marker type %1 is not available for channel '%2'; valid values: %3")
				    .arg(type)
				    .arg(channel, valid.join(", ")));
	}

	ch->applyMarkerType(type);
	return verify(QString("freq channel '%1' marker type").arg(channel), (int)mc->markerType(), type);
}

// ==================== FREQUENCY DOMAIN - CURSORS ====================

bool ADC_API::isFreqCursorsEnabled()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager) {
		fail("frequency instrument has no plots");
		return false;
	}
	auto *plot = ctrl->m_plotComponentManager->plot(0);
	if(!plot || !plot->cursor()) {
		fail("frequency plot has no cursor controller");
		return false;
	}
	return plot->cursor()->isVisible();
}

bool ADC_API::setFreqCursorsEnabled(bool enabled)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_ui || !ctrl->m_ui->m_cursor)
		return fail("frequency instrument has no cursor button");
	if(!ctrl->m_plotComponentManager)
		return fail("frequency instrument has no plots");
	auto *plot = ctrl->m_plotComponentManager->plot(0);
	if(!plot || !plot->cursor())
		return fail("frequency plot has no cursor controller");

	ctrl->m_ui->m_cursor->setChecked(enabled);
	return verify("freq cursors", plot->cursor()->isVisible(), enabled);
}

// ==================== AUTOSCALE ====================

bool ADC_API::isFreqAutoscaleEnabled()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager) {
		fail("frequency instrument has no plots");
		return false;
	}
	auto *plot = dynamic_cast<FFTPlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu()) {
		fail("frequency plot has no settings menu");
		return false;
	}
	return plot->plotMenu()->m_autoscaleEnabled;
}

bool ADC_API::setFreqAutoscaleEnabled(bool enabled)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager)
		return fail("frequency instrument has no plots");
	auto *plot = dynamic_cast<FFTPlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu())
		return fail("frequency plot has no settings menu");
	auto *menu = plot->plotMenu();
	if(!menu->m_autoscaleBtn || !menu->m_autoscaleBtn->onOffswitch())
		return fail("frequency plot has no autoscale switch");

	menu->m_autoscaleBtn->onOffswitch()->setChecked(enabled);
	return verify("freq autoscale", menu->m_autoscaleEnabled, enabled);
}

// ==================== X-MODE ====================

int ADC_API::getFreqXMode()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return -1;
	if(!ctrl->m_fftPlotSettingsComponent->m_xModeCb)
		return failInt("frequency instrument has no X-mode combo");
	return static_cast<int>(ctrl->m_fftPlotSettingsComponent->xMode());
}

bool ADC_API::setFreqXMode(int mode)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	auto *settings = ctrl->m_fftPlotSettingsComponent;
	if(!settings->m_xModeCb)
		return fail("frequency instrument has no X-mode combo");

	// XMODE_TIME is only inserted once a channel reports a sample rate, so the set of
	// valid values is runtime dependent - setXMode() rejects the absent ones for us.
	if(!settings->setXMode(static_cast<FFTPlotManagerSettings::XMode>(mode)))
		return fail(QString("invalid freq X-mode %1; valid values: %2")
				    .arg(mode)
				    .arg(comboValues(settings->m_xModeCb->combo())));
	return verify("freq X-mode", static_cast<int>(settings->xMode()), mode);
}

// ==================== Y-MODE ====================

int ADC_API::getTimeYMode()
{
	clearError();
	auto *menu = getTimePlotSettings();
	if(!menu)
		return -1;
	if(!menu->m_yModeCb)
		return failInt("time instrument has no Y-mode combo");
	// yMode() is the mode actually in force on the axis; the combo is kept in sync with it.
	return static_cast<int>(menu->yMode());
}

bool ADC_API::setTimeYMode(int mode)
{
	clearError();
	auto *menu = getTimePlotSettings();
	if(!menu)
		return false;
	if(!menu->m_yModeCb)
		return fail("time instrument has no Y-mode combo");

	// YMODE_SCALE is only appended once every channel reports a scale attribute, so the
	// valid set is runtime dependent - setYMode() rejects the absent ones for us. It also
	// applies when the combo already sits on mode, which driving the combo would not.
	if(!menu->setYMode(static_cast<YMode>(mode)))
		return fail(QString("invalid Y-mode %1; valid values: %2")
				    .arg(mode)
				    .arg(comboValues(menu->m_yModeCb->combo())));
	return verify("Y-mode", static_cast<int>(menu->yMode()), mode);
}

// ==================== CHANNEL AVERAGING (FFT) ====================

bool ADC_API::isFreqChannelAveragingEnabled(const QString &channel)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return false;
	int navg = -1;
	if(auto *complexSigpath = dynamic_cast<GRFFTComplexChannelSigpath *>(ch->m_grtch))
		navg = complexSigpath->averagingSize();
	else if(auto *floatSigpath = dynamic_cast<GRFFTChannelSigpath *>(ch->m_grtch))
		navg = floatSigpath->averagingSize();
	if(navg < 0) {
		fail(QString("freq channel '%1' has no FFT signal path").arg(channel));
		return false;
	}
	// The signal path spells "averaging off" as a single average.
	return navg > 1;
}

bool ADC_API::setFreqChannelAveragingEnabled(const QString &channel, bool enabled)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return false;
	if(!ch->m_avgSection || !ch->m_avgSpin)
		return fail(QString("freq channel '%1' has no averaging menu").arg(channel));

	// GRFFTChannelComponent::setAveragingSize() is a deliberate no-op - averaging can
	// only be changed from inside the channel's signal path - so go through the AVERAGING
	// section header instead. Its existing connection pushes the size (the spinbox value
	// when on, 1 when off) to the signal path, which is the state that matters. The
	// spinbox itself cannot express "off": its range starts at 2, hence enabling always
	// applies whatever the spinbox currently holds (2 by default, or the last size set).
	ch->applyAveragingEnabled(enabled);

	int navg = -1;
	if(auto *complexSigpath = dynamic_cast<GRFFTComplexChannelSigpath *>(ch->m_grtch))
		navg = complexSigpath->averagingSize();
	else if(auto *floatSigpath = dynamic_cast<GRFFTChannelSigpath *>(ch->m_grtch))
		navg = floatSigpath->averagingSize();
	if(navg < 0)
		return fail(QString("freq channel '%1' has no FFT signal path").arg(channel));

	// Both the header and the signal path must agree, or the menu would lie about what
	// the FFT is doing.
	if(ch->m_avgSection->collapseSection()->header()->isChecked() != enabled)
		return fail(QString("freq channel '%1' averaging: the AVERAGING section did not toggle").arg(channel));
	return verify(QString("freq channel '%1' averaging").arg(channel), navg > 1, enabled);
}

int ADC_API::getFreqChannelAveragingSize(const QString &channel)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return -1;
	if(auto *complexSigpath = dynamic_cast<GRFFTComplexChannelSigpath *>(ch->m_grtch))
		return complexSigpath->averagingSize();
	if(auto *floatSigpath = dynamic_cast<GRFFTChannelSigpath *>(ch->m_grtch))
		return floatSigpath->averagingSize();
	return failInt(QString("freq channel '%1' has no FFT signal path").arg(channel));
}

bool ADC_API::setFreqChannelAveragingSize(const QString &channel, int size)
{
	clearError();
	auto *ch = findFreqChannel(channel);
	if(!ch)
		return false;
	if(!ch->m_avgSection || !ch->m_avgSpin)
		return fail(QString("freq channel '%1' has no averaging menu").arg(channel));

	QSpinBox *spin = ch->m_avgSpin;
	if(size < spin->minimum() || size > spin->maximum())
		return fail(QString("freq averaging size %1 is out of range [%2, %3]; use "
				    "setFreqChannelAveragingEnabled(false) to turn averaging off")
				    .arg(size)
				    .arg(spin->minimum())
				    .arg(spin->maximum()));
	// The size spinbox pushes to the signal path unconditionally, without consulting the
	// section header. Writing it while averaging is off would leave averagingSize() > 1
	// under an unchecked AVERAGING header - the menu would read "off" while the FFT
	// averaged. Require the caller to enable averaging first.
	if(!ch->m_avgSection->collapseSection()->header()->isChecked())
		return fail(QString("averaging is disabled on '%1'; call "
				    "setFreqChannelAveragingEnabled('%1', true) first")
				    .arg(channel));

	// Same reason as above: drive the size spinbox, whose connection is what actually
	// reaches FFTChannel::setAveragingSize() on the signal path.
	ch->applyAveragingSize(size);

	int navg = -1;
	if(auto *complexSigpath = dynamic_cast<GRFFTComplexChannelSigpath *>(ch->m_grtch))
		navg = complexSigpath->averagingSize();
	else if(auto *floatSigpath = dynamic_cast<GRFFTChannelSigpath *>(ch->m_grtch))
		navg = floatSigpath->averagingSize();
	if(navg < 0)
		return fail(QString("freq channel '%1' has no FFT signal path").arg(channel));

	return verify(QString("freq channel '%1' averaging size").arg(channel), navg, size);
}

// ==================== WATERFALL ====================

bool ADC_API::isWaterfallEnabled()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager) {
		fail("frequency instrument has no plots");
		return false;
	}
	auto *plot = dynamic_cast<FFTPlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu() || !plot->plotMenu()->m_waterfallSwitch) {
		fail("frequency plot has no waterfall switch");
		return false;
	}
	return plot->plotMenu()->m_waterfallSwitch->isChecked();
}

bool ADC_API::setWaterfallEnabled(bool enabled)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager)
		return fail("frequency instrument has no plots");
	auto *plot = dynamic_cast<FFTPlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu() || !plot->plotMenu()->m_waterfallSwitch)
		return fail("frequency plot has no waterfall switch");

	// Drive the GUI switch so the menu, dock wrapper and plot stay in sync.
	QAbstractButton *sw = plot->plotMenu()->m_waterfallSwitch;
	sw->setChecked(enabled);
	return verify("waterfall enabled", sw->isChecked(), enabled);
}

int ADC_API::getWaterfallHistory()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return -1;
	if(!ctrl->m_plotComponentManager)
		return failInt("frequency instrument has no plots");
	auto *plot = dynamic_cast<FFTPlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu() || !plot->plotMenu()->m_waterfallRows)
		return failInt("frequency plot has no waterfall history spinbox");
	return (int)plot->plotMenu()->m_waterfallRows->value();
}

bool ADC_API::setWaterfallHistory(int rows)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_plotComponentManager)
		return fail("frequency instrument has no plots");
	auto *plot = dynamic_cast<FFTPlotComponent *>(ctrl->m_plotComponentManager->plot(0));
	if(!plot || !plot->plotMenu() || !plot->plotMenu()->m_waterfallRows)
		return fail("frequency plot has no waterfall history spinbox");

	gui::MenuSpinbox *spin = plot->plotMenu()->m_waterfallRows;
	if(rows < spin->min() || rows > spin->max())
		return fail(QString("waterfall history %1 is out of range [%2, %3]")
				    .arg(rows)
				    .arg((int)spin->min())
				    .arg((int)spin->max()));

	spin->setValue(rows);
	return verify("waterfall history", (int)spin->value(), rows);
}

// ==================== GENALYZER ====================
//
// Every write here drives a GenalyzerSettings widget rather than calling
// GenalyzerSettings::setConfig(). setConfig() assigns m_config and refreshes the
// widgets with signals blocked, but it never emits configChanged() -- so no channel
// ever learns about the new config and the flowgraph keeps analysing with the old
// one. Touching the widgets makes onUIChanged() run, which rebuilds m_config from
// the UI *and* emits configChanged(). Do not "simplify" these back to setConfig().
//
// onUIChanged() is also mode-gated: it copies only the fields of the currently
// selected mode. A FIXED_TONE field written while AUTO is active drives the widget
// but never reaches m_config, so the parameter setters below require the matching
// mode up front instead of reporting a success the config would not back up.

bool ADC_API::isGenalyzerEnabled()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	if(!ctrl->m_genalyzerSettings)
		return fail("genalyzer settings not available on the Frequency instrument");
	return ctrl->m_genalyzerSettings->getConfig().enabled;
}

bool ADC_API::setGenalyzerEnabled(bool enabled)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	auto *gs = ctrl->m_genalyzerSettings;
	if(!gs)
		return fail("genalyzer settings not available on the Frequency instrument");
	if(!ctrl->m_ui || !ctrl->m_ui->m_analyze || !ctrl->m_ui->m_complex)
		return fail("Frequency instrument UI not built yet");

	// Genalyzer only analyses complex channels; the Genalyzer button is hidden while
	// the instrument is in real mode and complex mode forcibly unchecks it.
	if(enabled && !ctrl->m_ui->m_complex->isChecked())
		return fail("genalyzer requires complex mode; call setFreqComplexMode(true) first");

	// The enable path is the m_analyze button, exactly as in the GUI: its toggled()
	// connection calls GenalyzerSettings::enableAnalysis() (which DOES emit
	// configChanged) and shows the genalyzer panel. setConfig() does neither.
	ctrl->m_ui->m_analyze->setChecked(enabled);

	// setChecked() on an already-correct button emits nothing, so a config that drifted
	// out of sync would stay stale. Reconcile it the same way the connection does.
	if(gs->getConfig().enabled != enabled) {
		gs->enableAnalysis(enabled);
		if(ctrl->m_plotComponentManager)
			ctrl->m_plotComponentManager->enableGenalyzerPanel(enabled);
	}

	return verify("genalyzer enabled", gs->getConfig().enabled, enabled);
}

int ADC_API::getGenalyzerMode()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return -1;
	if(!ctrl->m_genalyzerSettings)
		return failInt("genalyzer settings not available on the Frequency instrument");
	return static_cast<int>(ctrl->m_genalyzerSettings->getConfig().mode);
}

bool ADC_API::setGenalyzerMode(int mode)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	auto *gs = ctrl->m_genalyzerSettings;
	if(!gs)
		return fail("genalyzer settings not available on the Frequency instrument");

	int idx = gs->m_modeCombo->findData(mode);
	if(idx < 0)
		return fail(QString("invalid genalyzer mode %1; expected 0 (AUTO) or 1 (FIXED_TONE)").arg(mode));

	// currentIndexChanged -> onUIChanged(): swaps the auto/fixed-tone containers and emits.
	gs->m_modeCombo->setCurrentIndex(idx);
	if(gs->m_modeCombo->currentData().toInt() != mode)
		return fail(QString("genalyzer mode combo rejected %1").arg(mode));
	// Setting a combo/spinbox to the value it already holds emits nothing, so a config
	// that drifted out of sync would stay stale. onUIChanged() re-derives m_config from
	// the widgets and emits configChanged; it is a private slot, reachable here only
	// because ADC_API is a friend of GenalyzerSettings (genalyzersettings.h:38).
	if(static_cast<int>(gs->getConfig().mode) != mode)
		gs->onUIChanged();

	return verify("genalyzer mode", static_cast<int>(gs->getConfig().mode), mode);
}

int ADC_API::getGenalyzerSSBWidth()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return -1;
	if(!ctrl->m_genalyzerSettings)
		return failInt("genalyzer settings not available on the Frequency instrument");
	return ctrl->m_genalyzerSettings->getConfig().auto_params.ssb_width;
}

bool ADC_API::setGenalyzerSSBWidth(int width)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	auto *gs = ctrl->m_genalyzerSettings;
	if(!gs)
		return fail("genalyzer settings not available on the Frequency instrument");

	// ssb_width is an AUTO-mode parameter; onUIChanged() ignores the widget in FIXED_TONE.
	if(gs->getConfig().mode != grutil::GenalyzerMode::AUTO)
		return fail("setGenalyzerSSBWidth requires AUTO mode (currently FIXED_TONE); "
			    "call setGenalyzerMode(0) first");

	auto *sb = gs->m_ssbWidthSpinbox;
	// m_config.auto_params.ssb_width is uint8_t and onUIChanged() casts to it, so
	// anything above 255 would truncate silently. Bound by whichever limit is tighter.
	const int lo = qMax(sb->minimum(), 0);
	const int hi = qMin(sb->maximum(), 255);
	if(width < lo || width > hi)
		return fail(QString("genalyzer SSB width %1 out of range %2..%3").arg(width).arg(lo).arg(hi));

	sb->setValue(width); // valueChanged -> onUIChanged()
	if(sb->value() != width)
		return fail(QString("genalyzer SSB width spinbox rejected %1, reads %2").arg(width).arg(sb->value()));
	if(gs->getConfig().auto_params.ssb_width != width)
		gs->onUIChanged(); // spinbox already held this value, so valueChanged never fired

	return verify("genalyzer SSB width", static_cast<int>(gs->getConfig().auto_params.ssb_width), width);
}

double ADC_API::getGenalyzerExpectedFreq()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return qQNaN();
	if(!ctrl->m_genalyzerSettings)
		return failDouble("genalyzer settings not available on the Frequency instrument");
	return ctrl->m_genalyzerSettings->getConfig().fixed_tone.expected_freq;
}

bool ADC_API::setGenalyzerExpectedFreq(double freq)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	auto *gs = ctrl->m_genalyzerSettings;
	if(!gs)
		return fail("genalyzer settings not available on the Frequency instrument");

	if(gs->getConfig().mode != grutil::GenalyzerMode::FIXED_TONE)
		return fail("setGenalyzerExpectedFreq requires FIXED_TONE mode (currently AUTO); "
			    "call setGenalyzerMode(1) first");

	if(!qIsFinite(freq))
		return fail("genalyzer expected frequency must be a finite number");
	// The line edit carries a QDoubleValidator(-1e9, 1e9); setText() bypasses it, so
	// range-check here to keep the field editable by hand afterwards.
	if(qAbs(freq) > 1e9)
		return fail(QString("genalyzer expected frequency %1 out of range -1e9..1e9").arg(freq));

	// onUIChanged() parses the text with a bare QString::toDouble(), so the text has to
	// round-trip exactly: 17 significant digits. Fixed notation would flush a small
	// frequency to 0.000000, and verify()'s relative epsilon would call that a success.
	gs->m_expectedFreqEdit->setText(QString::number(freq, 'g', 17));
	// Exact != is deliberate and safe here: 17 significant digits round-trip bit-for-bit
	// through toDouble(), so a surviving difference means textChanged never fired (the
	// text was already identical) rather than a formatting artefact. Guarding instead of
	// always emitting matters -- genalyzer_fft_vii_impl::set_config() takes a mutex and
	// discards the cached FA/auto configs, so a redundant emit is not free.
	if(gs->getConfig().fixed_tone.expected_freq != freq)
		gs->onUIChanged();

	return verify("genalyzer expected frequency", gs->getConfig().fixed_tone.expected_freq, freq);
}

int ADC_API::getGenalyzerHarmonicOrder()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return -1;
	if(!ctrl->m_genalyzerSettings)
		return failInt("genalyzer settings not available on the Frequency instrument");
	return ctrl->m_genalyzerSettings->getConfig().fixed_tone.harmonic_order;
}

bool ADC_API::setGenalyzerHarmonicOrder(int order)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	auto *gs = ctrl->m_genalyzerSettings;
	if(!gs)
		return fail("genalyzer settings not available on the Frequency instrument");

	if(gs->getConfig().mode != grutil::GenalyzerMode::FIXED_TONE)
		return fail("setGenalyzerHarmonicOrder requires FIXED_TONE mode (currently AUTO); "
			    "call setGenalyzerMode(1) first");

	// harmonic_order is a plain int in GenalyzerConfig, so the spinbox range is the
	// only bound that can be exceeded -- no narrowing cast to worry about.
	auto *sb = gs->m_harmonicOrderSpinbox;
	if(order < sb->minimum() || order > sb->maximum())
		return fail(QString("genalyzer harmonic order %1 out of range %2..%3")
				    .arg(order)
				    .arg(sb->minimum())
				    .arg(sb->maximum()));

	sb->setValue(order);
	if(sb->value() != order)
		return fail(QString("genalyzer harmonic order spinbox rejected %1, reads %2")
				    .arg(order)
				    .arg(sb->value()));
	if(gs->getConfig().fixed_tone.harmonic_order != order)
		gs->onUIChanged();

	return verify("genalyzer harmonic order", gs->getConfig().fixed_tone.harmonic_order, order);
}

int ADC_API::getGenalyzerSSBFundamental()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return -1;
	if(!ctrl->m_genalyzerSettings)
		return failInt("genalyzer settings not available on the Frequency instrument");
	return ctrl->m_genalyzerSettings->getConfig().fixed_tone.ssb_fundamental;
}

bool ADC_API::setGenalyzerSSBFundamental(int ssb)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	auto *gs = ctrl->m_genalyzerSettings;
	if(!gs)
		return fail("genalyzer settings not available on the Frequency instrument");

	if(gs->getConfig().mode != grutil::GenalyzerMode::FIXED_TONE)
		return fail("setGenalyzerSSBFundamental requires FIXED_TONE mode (currently AUTO); "
			    "call setGenalyzerMode(1) first");

	auto *sb = gs->m_ssbFundamentalSpinbox;
	if(ssb < sb->minimum() || ssb > sb->maximum())
		return fail(QString("genalyzer SSB fundamental %1 out of range %2..%3")
				    .arg(ssb)
				    .arg(sb->minimum())
				    .arg(sb->maximum()));

	sb->setValue(ssb);
	if(sb->value() != ssb)
		return fail(QString("genalyzer SSB fundamental spinbox rejected %1, reads %2")
				    .arg(ssb)
				    .arg(sb->value()));
	if(gs->getConfig().fixed_tone.ssb_fundamental != ssb)
		gs->onUIChanged();

	return verify("genalyzer SSB fundamental", gs->getConfig().fixed_tone.ssb_fundamental, ssb);
}

int ADC_API::getGenalyzerSSBDefault()
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return -1;
	if(!ctrl->m_genalyzerSettings)
		return failInt("genalyzer settings not available on the Frequency instrument");
	return ctrl->m_genalyzerSettings->getConfig().fixed_tone.ssb_default;
}

bool ADC_API::setGenalyzerSSBDefault(int ssb)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	auto *gs = ctrl->m_genalyzerSettings;
	if(!gs)
		return fail("genalyzer settings not available on the Frequency instrument");

	if(gs->getConfig().mode != grutil::GenalyzerMode::FIXED_TONE)
		return fail("setGenalyzerSSBDefault requires FIXED_TONE mode (currently AUTO); "
			    "call setGenalyzerMode(1) first");

	auto *sb = gs->m_ssbDefaultSpinbox;
	if(ssb < sb->minimum() || ssb > sb->maximum())
		return fail(QString("genalyzer SSB default %1 out of range %2..%3")
				    .arg(ssb)
				    .arg(sb->minimum())
				    .arg(sb->maximum()));

	sb->setValue(ssb);
	if(sb->value() != ssb)
		return fail(
			QString("genalyzer SSB default spinbox rejected %1, reads %2").arg(ssb).arg(sb->value()));
	if(gs->getConfig().fixed_tone.ssb_default != ssb)
		gs->onUIChanged();

	return verify("genalyzer SSB default", gs->getConfig().fixed_tone.ssb_default, ssb);
}

QStringList ADC_API::getGenalyzerMetrics(const QString &channel)
{
	clearError();
	QStringList metrics;
	auto *ch = findComplexChannel(channel);
	if(!ch)
		return metrics;
	auto *complexSigpath = dynamic_cast<GRFFTComplexChannelSigpath *>(ch->m_grtch);
	if(!complexSigpath) {
		fail(QString("channel '%1' has no complex signal path").arg(channel));
		return metrics;
	}
	gn_analysis_results *results = complexSigpath->getGnAnalysis();
	if(!results || !results->rkeys || !results->rvalues || results->results_size == 0) {
		// Metrics only exist once the genalyzer block has processed a buffer.
		fail(QString("no genalyzer results for channel '%1' yet; enable genalyzer "
			     "(setGenalyzerEnabled(true)) and run an acquisition first")
			     .arg(channel));
		return metrics;
	}
	for(size_t i = 0; i < results->results_size; ++i) {
		if(results->rkeys[i])
			metrics.append(QString(results->rkeys[i]));
	}
	return metrics;
}

double ADC_API::getGenalyzerMetric(const QString &channel, const QString &metric)
{
	clearError();
	auto *ch = findComplexChannel(channel);
	if(!ch)
		return qQNaN();
	auto *complexSigpath = dynamic_cast<GRFFTComplexChannelSigpath *>(ch->m_grtch);
	if(!complexSigpath)
		return failDouble(QString("channel '%1' has no complex signal path").arg(channel));
	gn_analysis_results *results = complexSigpath->getGnAnalysis();
	if(!results || !results->rkeys || !results->rvalues || results->results_size == 0)
		return failDouble(QString("no genalyzer results for channel '%1' yet; enable genalyzer "
					  "(setGenalyzerEnabled(true)) and run an acquisition first")
					 .arg(channel));

	QStringList available;
	for(size_t i = 0; i < results->results_size; ++i) {
		if(!results->rkeys[i])
			continue;
		if(metric == QString(results->rkeys[i]))
			return results->rvalues[i];
		available.append(QString(results->rkeys[i]));
	}
	return failDouble(QString("no such genalyzer metric '%1' on channel '%2'; available: %3")
				  .arg(metric, channel, available.join(", ")));
}

bool ADC_API::triggerGenalyzerAnalysis(const QString &channel)
{
	clearError();
	auto *ctrl = getFreqController();
	if(!ctrl)
		return false;
	auto *gs = ctrl->m_genalyzerSettings;
	if(!gs)
		return fail("genalyzer settings not available on the Frequency instrument");
	if(!gs->getConfig().enabled)
		return fail("genalyzer is not enabled; call setGenalyzerEnabled(true) first");

	auto *ch = findComplexChannel(channel);
	if(!ch)
		return false;
	if(!ch->enabled())
		return fail(QString("channel '%1' is disabled; call setFreqChannelEnabled('%1', true) first")
				    .arg(channel));

	// triggerGenalyzerAnalysis() only re-broadcasts results the genalyzer block has
	// already produced, so with no results there is nothing to report and claiming
	// success would be a lie.
	auto *complexSigpath = dynamic_cast<GRFFTComplexChannelSigpath *>(ch->m_grtch);
	if(!complexSigpath)
		return fail(QString("channel '%1' has no complex signal path").arg(channel));
	gn_analysis_results *results = complexSigpath->getGnAnalysis();
	if(!results || results->results_size == 0)
		return fail(QString("no genalyzer data available for channel '%1'; start the acquisition "
				    "(setFreqRunning(true)) and let at least one buffer complete")
				    .arg(channel));

	ch->triggerGenalyzerAnalysis();
	return true;
}

#include "moc_adc_api.cpp"
