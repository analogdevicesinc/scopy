/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 */

#include "adalm2000osc_api.h"

#include "horizontalsettings.h"
#include "m2koschistogramprocessor.h"
#include "m2kosccontroller.h"

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_ui/acqinstrument.h>
#include <core/acq_ui/acqplot.h>
#include <gui/cursorcontroller.h>
#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotcursors.h>

#include <QAbstractButton>

#include <cmath>

using namespace scopy;
using namespace scopy::adalm2000;
using scopy::acq::Measure;
using Controller = M2kOscController;

namespace {
constexpr int kDivs = 10;

int measureOf(const QString &id)
{
	for(int i = 0; i < scopy::acq::MEASURE_COUNT; ++i) {
		if(scopy::acq::measureId(static_cast<Measure>(i)) == id) {
			return i;
		}
	}
	return -1;
}

bool validChannel(int ch) { return ch >= 0 && ch < Controller::CHANNELS; }

QList<double> latest(const Controller *tool, const scopy::acq::DataKey &key)
{
	QList<double> out;
	for(float v : tool->ui()->store()->latestFloat(key).value_or(QVector<float>{})) {
		out.append(v);
	}
	return out;
}

template <class T, class F>
QList<T> perChannel(F get)
{
	QList<T> out;
	for(int ch = 0; ch < Controller::CHANNELS; ++ch) {
		out.append(get(ch));
	}
	return out;
}

template <class T, class F>
void forChannels(const QList<T> &values, F set)
{
	for(int ch = 0; ch < Controller::CHANNELS && ch < values.size(); ++ch) {
		set(ch, values[ch]);
	}
}

using IsShown = bool (Controller::*)(int, Measure) const;
using SetShown = void (Controller::*)(int, Measure, bool);

QStringList shown(const Controller *tool, IsShown is)
{
	QStringList out;
	for(int ch = 0; ch < tool->traceCount(); ++ch) {
		for(int i = 0; i < scopy::acq::MEASURE_COUNT; ++i) {
			if((tool->*is)(ch, static_cast<Measure>(i))) {
				out.append(QStringLiteral("%1:%2").arg(ch).arg(
					scopy::acq::measureId(static_cast<Measure>(i))));
			}
		}
	}
	return out;
}

void setShown(Controller *tool, SetShown set, const QStringList &list)
{
	for(int ch = 0; ch < tool->traceCount(); ++ch) {
		for(int i = 0; i < scopy::acq::MEASURE_COUNT; ++i) {
			const QString entry =
				QStringLiteral("%1:%2").arg(ch).arg(scopy::acq::measureId(static_cast<Measure>(i)));
			(tool->*set)(ch, static_cast<Measure>(i), list.contains(entry));
		}
	}
}
} // namespace

Adalm2000Osc_API::Adalm2000Osc_API(M2kOscController *tool)
	: ApiObject()
	, m_tool(tool)
{}

bool Adalm2000Osc_API::running() const { return m_tool->ui()->engine()->isRunning(); }

void Adalm2000Osc_API::setRunning(bool on)
{
	if(on) {
		m_tool->ui()->run();
	} else {
		m_tool->ui()->stop();
	}
}

double Adalm2000Osc_API::sampleRate() const { return m_tool->sweep().sampleRate; }

void Adalm2000Osc_API::setSampleRate(double hz)
{
	sweep::Sweep s = m_tool->sweep();
	s.sampleRate = hz;
	m_tool->setSweep(s);
}

int Adalm2000Osc_API::bufferSize() const { return m_tool->sweep().bufferSize; }

void Adalm2000Osc_API::setBufferSize(int n)
{
	m_tool->setSweep(sweep::withBuffer(m_tool->sweep(), n, m_tool->m_limits));
}

int Adalm2000Osc_API::delay() const { return m_tool->sweep().delay; }

void Adalm2000Osc_API::setDelay(int samples)
{
	sweep::Sweep s = m_tool->sweep();
	s.delay = samples;
	m_tool->setSweep(s);
}

double Adalm2000Osc_API::timeBase() const { return m_tool->sweep().timeBase(kDivs); }

void Adalm2000Osc_API::setTimeBase(double secPerDiv)
{
	m_tool->setSweep(sweep::fromTimeBase(secPerDiv, timePosition(), kDivs, m_tool->m_limits));
}

double Adalm2000Osc_API::timePosition() const { return m_tool->sweep().position(); }

void Adalm2000Osc_API::setTimePosition(double s)
{
	m_tool->setSweep(sweep::fromTimeBase(timeBase(), s, kDivs, m_tool->m_limits));
}

int Adalm2000Osc_API::timeBaseSamples() const { return m_tool->m_horizontal->timeBaseSamples(); }

void Adalm2000Osc_API::setTimeBaseSamples(int n) { m_tool->m_horizontal->setTimeBaseSamples(n); }

QList<bool> Adalm2000Osc_API::enabled() const
{
	return perChannel<bool>([this](int ch) { return m_tool->isChannelEnabled(ch); });
}

void Adalm2000Osc_API::setEnabled(const QList<bool> &on)
{
	forChannels(on, [this](int ch, bool v) { m_tool->setChannelEnabled(ch, v); });
}

QList<double> Adalm2000Osc_API::voltsPerDiv() const
{
	return perChannel<double>([this](int ch) { return m_tool->voltsPerDiv(ch); });
}

void Adalm2000Osc_API::setVoltsPerDiv(const QList<double> &v)
{
	forChannels(v, [this](int ch, double x) { m_tool->setVoltsPerDiv(ch, x); });
}

QList<double> Adalm2000Osc_API::verticalPosition() const
{
	return perChannel<double>([this](int ch) { return m_tool->position(ch); });
}

void Adalm2000Osc_API::setVerticalPosition(const QList<double> &v)
{
	forChannels(v, [this](int ch, double x) { m_tool->setPosition(ch, x); });
}

QList<double> Adalm2000Osc_API::probe() const
{
	return perChannel<double>([this](int ch) { return m_tool->probe(ch); });
}

void Adalm2000Osc_API::setProbe(const QList<double> &v)
{
	forChannels(v, [this](int ch, double x) { m_tool->setProbe(ch, x); });
}

QList<bool> Adalm2000Osc_API::acCoupled() const
{
	return perChannel<bool>([this](int ch) { return m_tool->isAcCoupled(ch); });
}

void Adalm2000Osc_API::setAcCoupled(const QList<bool> &on)
{
	forChannels(on, [this](int ch, bool v) { m_tool->setAcCoupled(ch, v); });
}

namespace {
QList<double> positions(PlotAxisHandle *a, PlotAxisHandle *b) { return {a->getPosition(), b->getPosition()}; }

void place(PlotAxisHandle *a, PlotAxisHandle *b, const QList<double> &pos)
{
	if(pos.size() == 2) {
		a->setPosition(pos[0]);
		b->setPosition(pos[1]);
	}
}
} // namespace

bool Adalm2000Osc_API::cursorsX() const { return m_tool->m_cursorXEn->isChecked(); }

void Adalm2000Osc_API::setCursorsX(bool on) { m_tool->m_cursorXEn->setChecked(on); }

bool Adalm2000Osc_API::cursorsY() const { return m_tool->m_cursorYEn->isChecked(); }

void Adalm2000Osc_API::setCursorsY(bool on) { m_tool->m_cursorYEn->setChecked(on); }

QList<double> Adalm2000Osc_API::cursorX() const
{
	return positions(m_tool->plotCursors()->getX1Cursor(), m_tool->plotCursors()->getX2Cursor());
}

void Adalm2000Osc_API::setCursorX(const QList<double> &pos)
{
	place(m_tool->plotCursors()->getX1Cursor(), m_tool->plotCursors()->getX2Cursor(), pos);
}

QList<double> Adalm2000Osc_API::cursorY() const
{
	return positions(m_tool->plotCursors()->getY1Cursor(), m_tool->plotCursors()->getY2Cursor());
}

void Adalm2000Osc_API::setCursorY(const QList<double> &pos)
{
	place(m_tool->plotCursors()->getY1Cursor(), m_tool->plotCursors()->getY2Cursor(), pos);
}

#define TRIGGER_FIELD(Type, getter, setter, field)                                                                     \
	Type Adalm2000Osc_API::getter() const { return m_tool->trigger().field; }                                      \
	void Adalm2000Osc_API::setter(Type v)                                                                          \
	{                                                                                                              \
		osc::Trigger t = m_tool->trigger();                                                                    \
		t.field = v;                                                                                           \
		m_tool->setTrigger(t);                                                                                 \
	}

TRIGGER_FIELD(bool, triggerEnabled, setTriggerEnabled, enabled)
TRIGGER_FIELD(bool, triggerAuto, setTriggerAuto, autoMode)
TRIGGER_FIELD(bool, triggerAnalog, setTriggerAnalog, analog)
TRIGGER_FIELD(int, triggerChannel, setTriggerChannel, channel)
TRIGGER_FIELD(int, triggerDigital, setTriggerDigital, digital)
TRIGGER_FIELD(int, triggerLogic, setTriggerLogic, logic)
TRIGGER_FIELD(int, triggerCondition, setTriggerCondition, condition)
TRIGGER_FIELD(int, triggerExternalCondition, setTriggerExternalCondition, externalCondition)
TRIGGER_FIELD(double, triggerLevel, setTriggerLevel, level)
TRIGGER_FIELD(double, triggerHysteresis, setTriggerHysteresis, hysteresis)
TRIGGER_FIELD(int, triggerOut, setTriggerOut, outSelect)

#undef TRIGGER_FIELD

QStringList Adalm2000Osc_API::math() const { return m_tool->mathFormulas(); }

void Adalm2000Osc_API::setMath(const QStringList &formulas)
{
	for(int i = m_tool->traceCount() - 1; i >= Controller::CHANNELS; --i) {
		if(m_tool->m_traces[i]->kind == Controller::Kind::Math) {
			m_tool->removeTrace(i);
		}
	}
	for(const QString &f : formulas) {
		m_tool->addMath(f);
	}
}

QStringList Adalm2000Osc_API::measurements() const { return shown(m_tool, &Controller::isMeasured); }

void Adalm2000Osc_API::setMeasurements(const QStringList &list) { setShown(m_tool, &Controller::setMeasured, list); }

QStringList Adalm2000Osc_API::measurementStats() const { return shown(m_tool, &Controller::isStatsShown); }

void Adalm2000Osc_API::setMeasurementStats(const QStringList &list)
{
	setShown(m_tool, &Controller::setStatsShown, list);
}

bool Adalm2000Osc_API::gated() const { return m_tool->isGated(); }

void Adalm2000Osc_API::setGated(bool on) { m_tool->setGated(on); }

bool Adalm2000Osc_API::xyEnabled() const { return m_tool->isViewEnabled(Controller::ViewXY); }

void Adalm2000Osc_API::setXYEnabled(bool on) { m_tool->setViewEnabled(Controller::ViewXY, on); }

QList<int> Adalm2000Osc_API::xySources() const { return {m_tool->xySource(0), m_tool->xySource(1)}; }

void Adalm2000Osc_API::setXYSources(const QList<int> &traces)
{
	if(traces.size() == 2) {
		m_tool->setXYSources(traces[0], traces[1]);
	}
}

bool Adalm2000Osc_API::histogramEnabled() const { return m_tool->isViewEnabled(Controller::ViewHistogram); }

void Adalm2000Osc_API::setHistogramEnabled(bool on) { m_tool->setViewEnabled(Controller::ViewHistogram, on); }

bool Adalm2000Osc_API::fftEnabled() const { return m_tool->isViewEnabled(Controller::ViewFFT); }

void Adalm2000Osc_API::setFftEnabled(bool on) { m_tool->setViewEnabled(Controller::ViewFFT, on); }

QList<double> Adalm2000Osc_API::fft(int ch) const
{
	return validChannel(ch) ? latest(m_tool, m_tool->fftKey(ch, QStringLiteral("fft"))) : QList<double>{};
}

QList<double> Adalm2000Osc_API::fftFrequencies(int ch) const
{
	return validChannel(ch) ? latest(m_tool, m_tool->fftKey(ch, QStringLiteral("fftf"))) : QList<double>{};
}

void Adalm2000Osc_API::single() { m_tool->single(); }

void Adalm2000Osc_API::autoset() { m_tool->autoset(); }

bool Adalm2000Osc_API::isAutosetting() const { return m_tool->isAutosetting(); }

QList<double> Adalm2000Osc_API::samples(int ch) const
{
	return validChannel(ch) ? latest(m_tool, m_tool->probedKey(ch)) : QList<double>{};
}

QList<double> Adalm2000Osc_API::time() const { return latest(m_tool, m_tool->m_source->timeKey()); }

QList<double> Adalm2000Osc_API::histogram(int ch) const
{
	return validChannel(ch) ? latest(m_tool, m_tool->m_hist->countsKey(ch)) : QList<double>{};
}

bool Adalm2000Osc_API::exportData(const QString &path) const { return m_tool->exportData(path); }

double Adalm2000Osc_API::measurement(int trace, const QString &id) const
{
	const int m = measureOf(id);
	return m >= 0 ? m_tool->measurement(trace, static_cast<Measure>(m)) : std::nan("");
}

QVariantMap Adalm2000Osc_API::statistics(int trace, const QString &id) const
{
	const int m = measureOf(id);
	return m >= 0 ? m_tool->statistics(trace, static_cast<Measure>(m)) : QVariantMap{};
}

int Adalm2000Osc_API::traceCount() const { return m_tool->traceCount(); }

QString Adalm2000Osc_API::traceName(int trace) const { return m_tool->traceName(trace); }

int Adalm2000Osc_API::addMath(const QString &formula) { return m_tool->addMath(formula); }

int Adalm2000Osc_API::snapshot(int trace) { return m_tool->snapshot(trace); }

int Adalm2000Osc_API::importReferences(const QString &path) { return m_tool->importReferences(path); }

void Adalm2000Osc_API::removeTrace(int trace) { m_tool->removeTrace(trace); }

#include "moc_adalm2000osc_api.cpp"
