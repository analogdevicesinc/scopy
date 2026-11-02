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

#include "adalm2000spectrum_api.h"

#include "m2kspectrumcontroller.h"

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_ui/acqcurvechannel.h>
#include <core/acq_ui/acqinstrument.h>
#include <core/acq_ui/acqmarkercontroller.h>

#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuspinbox.h>

#include <QAbstractButton>
#include <QComboBox>
#include <QVariantMap>

using namespace scopy;
using namespace scopy::adalm2000;
using Step = scopy::adc::AcqMarkerController::PeakStep;

namespace {
template <typename T, typename Get>
QList<T> perChannel(Get get)
{
	QList<T> out;
	for(int ch = 0; ch < M2kSpectrumController::CHANNELS; ++ch) {
		out.append(get(ch));
	}
	return out;
}

template <typename T, typename Set>
void forChannels(const QList<T> &values, Set set)
{
	for(int ch = 0; ch < M2kSpectrumController::CHANNELS && ch < values.size(); ++ch) {
		set(ch, values[ch]);
	}
}

QList<double> toDoubles(const std::optional<QVector<float>> &v)
{
	QList<double> out;
	for(float f : v.value_or(QVector<float>{})) {
		out.append(f);
	}
	return out;
}
} // namespace

Adalm2000Spectrum_API::Adalm2000Spectrum_API(M2kSpectrumController *tool)
	: ApiObject()
	, m_tool(tool)
{}

bool Adalm2000Spectrum_API::running() const { return m_tool->ui()->engine()->isRunning(); }

void Adalm2000Spectrum_API::setRunning(bool on)
{
	if(on) {
		m_tool->ui()->run();
	} else {
		m_tool->ui()->stop();
	}
}

int Adalm2000Spectrum_API::fftSize() const { return m_tool->m_fftSize; }

void Adalm2000Spectrum_API::setFftSize(int n)
{
	const int idx = spec::fftSizes().indexOf(n);
	if(idx >= 0) {
		m_tool->m_rbwCombo->combo()->setCurrentIndex(idx);
	}
}

double Adalm2000Spectrum_API::startFreq() const { return m_tool->m_span.start; }

void Adalm2000Spectrum_API::setStartFreq(double hz) { m_tool->m_startSpin->setValue(hz); }

double Adalm2000Spectrum_API::stopFreq() const { return m_tool->m_span.stop; }

void Adalm2000Spectrum_API::setStopFreq(double hz) { m_tool->m_stopSpin->setValue(hz); }

bool Adalm2000Spectrum_API::logFreq() const { return m_tool->m_logX; }

void Adalm2000Spectrum_API::setLogFreq(bool on) { m_tool->m_logSwitch->onOffswitch()->setChecked(on); }

int Adalm2000Spectrum_API::units() const { return m_tool->m_unitsCombo->combo()->currentIndex(); }

void Adalm2000Spectrum_API::setUnits(int idx) { m_tool->m_unitsCombo->combo()->setCurrentIndex(idx); }

double Adalm2000Spectrum_API::top() const { return m_tool->m_topSpin->value(); }

void Adalm2000Spectrum_API::setTop(double v) { m_tool->m_topSpin->setValue(v); }

double Adalm2000Spectrum_API::bottom() const { return m_tool->m_bottomSpin->value(); }

void Adalm2000Spectrum_API::setBottom(double v) { m_tool->m_bottomSpin->setValue(v); }

QList<bool> Adalm2000Spectrum_API::enabled() const
{
	return perChannel<bool>([this](int ch) { return m_tool->m_chEnabled[ch]; });
}

void Adalm2000Spectrum_API::setEnabled(const QList<bool> &on)
{
	forChannels(on, [this](int ch, bool v) { m_tool->setChannelEnabled(ch, v); });
}

QList<int> Adalm2000Spectrum_API::averaging() const
{
	return perChannel<int>([this](int ch) { return m_tool->m_chUi[ch].averaging->combo()->currentIndex(); });
}

void Adalm2000Spectrum_API::setAveraging(const QList<int> &modes)
{
	forChannels(modes, [this](int ch, int v) { m_tool->m_chUi[ch].averaging->combo()->setCurrentIndex(v); });
}

QList<int> Adalm2000Spectrum_API::history() const
{
	return perChannel<int>([this](int ch) { return static_cast<int>(m_tool->m_chUi[ch].history->value()); });
}

void Adalm2000Spectrum_API::setHistory(const QList<int> &frames)
{
	forChannels(frames, [this](int ch, int v) { m_tool->m_chUi[ch].history->setValue(v); });
}

QList<int> Adalm2000Spectrum_API::window() const
{
	return perChannel<int>([this](int ch) { return m_tool->m_chUi[ch].window->combo()->currentIndex(); });
}

void Adalm2000Spectrum_API::setWindow(const QList<int> &windows)
{
	forChannels(windows, [this](int ch, int v) { m_tool->m_chUi[ch].window->combo()->setCurrentIndex(v); });
}

QList<bool> Adalm2000Spectrum_API::lowRange() const
{
	return perChannel<bool>([this](int ch) { return m_tool->m_chUi[ch].range->combo()->currentIndex() == 1; });
}

void Adalm2000Spectrum_API::setLowRange(const QList<bool> &low)
{
	forChannels(low, [this](int ch, bool v) { m_tool->m_chUi[ch].range->combo()->setCurrentIndex(v ? 1 : 0); });
}

bool Adalm2000Spectrum_API::waterfall() const { return m_tool->m_waterfallOn; }

void Adalm2000Spectrum_API::setWaterfall(bool on) { m_tool->m_wfSwitch->onOffswitch()->setChecked(on); }

int Adalm2000Spectrum_API::waterfallChannel() const { return m_tool->m_waterfallCh; }

void Adalm2000Spectrum_API::setWaterfallChannel(int ch) { m_tool->m_wfChannelCombo->combo()->setCurrentIndex(ch); }

int Adalm2000Spectrum_API::markerType() const { return m_tool->m_markerTypeCombo->combo()->currentIndex(); }

void Adalm2000Spectrum_API::setMarkerType(int idx) { m_tool->m_markerTypeCombo->combo()->setCurrentIndex(idx); }

int Adalm2000Spectrum_API::markerCount() const { return static_cast<int>(m_tool->m_markerCountSpin->value()); }

void Adalm2000Spectrum_API::setMarkerCount(int n) { m_tool->m_markerCountSpin->setValue(n); }

void Adalm2000Spectrum_API::single() { m_tool->single(); }

void Adalm2000Spectrum_API::snapshot(int ch)
{
	if(ch >= 0 && ch < M2kSpectrumController::CHANNELS) {
		m_tool->snapshot(ch);
	}
}

QList<double> Adalm2000Spectrum_API::spectrum(int ch) const
{
	return toDoubles(m_tool->ui()->store()->latestFloat(M2kSpectrumController::spectrumKey(ch)));
}

QList<double> Adalm2000Spectrum_API::frequencies(int ch) const
{
	return toDoubles(m_tool->ui()->store()->latestFloat(M2kSpectrumController::freqKey(ch)));
}

QVariantList Adalm2000Spectrum_API::markers(int ch) const
{
	QVariantList out;
	if(ch < 0 || ch >= M2kSpectrumController::CHANNELS) {
		return out;
	}
	for(const auto &m : m_tool->m_markers[ch]->markerInfo()) {
		out.append(QVariantMap{{"name", m.name}, {"frequency", m.peak.x}, {"value", m.peak.y}});
	}
	return out;
}

void Adalm2000Spectrum_API::setMarkerFrequency(int ch, int idx, double hz)
{
	if(ch >= 0 && ch < M2kSpectrumController::CHANNELS) {
		m_tool->m_markers[ch]->moveFixedMarker(idx, hz);
	}
}

bool Adalm2000Spectrum_API::stepMarker(int ch, int idx, const QString &step)
{
	static const QMap<QString, Step> steps = {{"left", Step::Left},
						  {"right", Step::Right},
						  {"higher", Step::Higher},
						  {"lower", Step::Lower},
						  {"max", Step::Highest}};
	if(ch < 0 || ch >= M2kSpectrumController::CHANNELS || !steps.contains(step)) {
		return false;
	}
	return m_tool->m_markers[ch]->stepToPeak(idx, steps.value(step));
}

#include "moc_adalm2000spectrum_api.cpp"
