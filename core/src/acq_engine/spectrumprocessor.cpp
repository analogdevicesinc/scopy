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
 *
 */

#include "spectrumprocessor.h"

#include "acquisitionengine.h"
#include "datastore.h"
#include "genalyzerfftprocessor.h"

#include <gui/style.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuspinbox.h>

#include <QMutexLocker>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

using namespace scopy;
using namespace scopy::acq;

namespace {

// dBu is referenced to 0.775 Vrms (1 mW into 600 ohm).
constexpr double kDbuRefVrms = 0.7745966692;

// Below anything a 12-bit converter can show; keeps log10(0) out of the power domain.
constexpr float kFloorDb = -300.0f;

inline double dbToPower(float db) { return std::pow(10.0, static_cast<double>(db) / 10.0); }
inline float powerToDb(double p) { return p > 0.0 ? static_cast<float>(10.0 * std::log10(p)) : kFloorDb; }

} // namespace

SpectrumProcessor::SpectrumProcessor(const QString &name, const DataKey &inputKey, const DataKey &outputKey,
				     const GenalyzerFFTProcessor *fft, QObject *parent)
	: ProcessorBlock(name, parent)
	, m_inputKey(inputKey)
	, m_outputKey(outputKey)
	, m_fft(fft)
{
	storeWatchedKeys({inputKey});
}

void SpectrumProcessor::setAveraging(Averaging mode)
{
	{
		QMutexLocker lk(&m_mutex);
		if(m_averaging == mode) {
			return;
		}
		m_averaging = mode;
		m_historyStale = true;
	}
	Q_EMIT averagingChanged(mode);
}

SpectrumProcessor::Averaging SpectrumProcessor::averaging() const
{
	QMutexLocker lk(&m_mutex);
	return m_averaging;
}

void SpectrumProcessor::setHistory(int frames)
{
	frames = std::max(1, frames);
	{
		QMutexLocker lk(&m_mutex);
		if(m_history == frames) {
			return;
		}
		m_history = frames;
		m_historyStale = true;
	}
	Q_EMIT historyChanged(frames);
}

int SpectrumProcessor::history() const
{
	QMutexLocker lk(&m_mutex);
	return m_history;
}

void SpectrumProcessor::setUnit(Unit unit)
{
	{
		QMutexLocker lk(&m_mutex);
		if(m_unit == unit) {
			return;
		}
		m_unit = unit;
	}
	Q_EMIT unitChanged(unit);
}

SpectrumProcessor::Unit SpectrumProcessor::unit() const
{
	QMutexLocker lk(&m_mutex);
	return m_unit;
}

void SpectrumProcessor::setFullScaleVolts(double volts)
{
	QMutexLocker lk(&m_mutex);
	m_fullScaleVolts = volts;
}

double SpectrumProcessor::fullScaleVolts() const
{
	QMutexLocker lk(&m_mutex);
	return m_fullScaleVolts;
}

QStringList SpectrumProcessor::averagingNames()
{
	return {QStringLiteral("Sample"),	 QStringLiteral("Peak hold continuous"),
		QStringLiteral("Peak hold"),	 QStringLiteral("Min hold continuous"),
		QStringLiteral("Min hold"),	 QStringLiteral("Linear RMS"),
		QStringLiteral("Linear dB"),	 QStringLiteral("Exponential RMS"),
		QStringLiteral("Exponential dB")};
}

QStringList SpectrumProcessor::unitNames()
{
	return {QStringLiteral("dBFS"),	 QStringLiteral("dBV"),	 QStringLiteral("dBu"),
		QStringLiteral("Vpeak"), QStringLiteral("Vrms"), QStringLiteral("V/√Hz")};
}

QString SpectrumProcessor::unitSymbol(Unit unit)
{
	switch(unit) {
	case Unit::DBFS:
		return QStringLiteral("dBFS");
	case Unit::DBV:
		return QStringLiteral("dBV");
	case Unit::DBU:
		return QStringLiteral("dBu");
	case Unit::VPeak:
	case Unit::VRms:
		return QStringLiteral("V");
	case Unit::VRmsPerSqrtHz:
		return QStringLiteral("V/√Hz");
	}
	return {};
}

bool SpectrumProcessor::usesHistory(Averaging mode)
{
	return mode != Averaging::Sample && mode != Averaging::PeakHoldContinuous &&
		mode != Averaging::MinHoldContinuous;
}

double SpectrumProcessor::enbwBins(int gnWindow)
{
	switch(gnWindow) {
	case GnWindowHann:
		return 1.5;
	case GnWindowBlackmanHarris:
		// 4-term Blackman-Harris.
		return 2.0044;
	default:
		return 1.0;
	}
}

std::optional<StreamInfo> SpectrumProcessor::streamInfo(const DataKey &key) const
{
	if(key != m_outputKey) {
		return std::nullopt;
	}
	StreamInfo info;
	info.label = m_outputKey.channelId();
	info.unit = unitSymbol(unit());
	info.kind = ReprKind::Curve;
	if(m_fft) {
		info.xKey = m_fft->freqKey();
		info.sampleRate = m_fft->sampleRate();
	} else if(auto *eng = qobject_cast<AcquisitionEngine *>(parent())) {
		if(const std::optional<StreamInfo> in = eng->streamInfo(m_inputKey)) {
			info.xKey = in->xKey;
			info.sampleRate = in->sampleRate;
		}
	}
	return info;
}

void SpectrumProcessor::reset()
{
	QMutexLocker lk(&m_mutex);
	m_historyStale = true;
}

void SpectrumProcessor::clearHistoryLocked()
{
	m_frames.clear();
	m_acc.clear();
	m_historyStale = false;
}

void SpectrumProcessor::process(DataStore *store)
{
	const std::optional<QVector<float>> in = store->latestFloat(m_inputKey);
	if(!in || in->isEmpty()) {
		return;
	}

	Averaging mode;
	int depth;
	Unit unit;
	double fullScale;
	{
		QMutexLocker lk(&m_mutex);
		if(m_historyStale) {
			clearHistoryLocked();
		}
		mode = m_averaging;
		depth = m_history;
		unit = m_unit;
		fullScale = m_fullScaleVolts;
	}

	// A new RBW changes the frame length; frames of two lengths cannot be combined.
	if(!m_acc.isEmpty() && m_acc.size() != in->size()) {
		m_frames.clear();
		m_acc.clear();
	}
	if(!m_frames.empty() && m_frames.back().size() != in->size()) {
		m_frames.clear();
	}

	store->write(m_outputKey, convert(average(*in, mode, depth), unit, fullScale));
}

QVector<float> SpectrumProcessor::average(const QVector<float> &db, Averaging mode, int depth)
{
	const int n = db.size();

	switch(mode) {
	case Averaging::Sample:
		return db;

	case Averaging::PeakHoldContinuous:
	case Averaging::MinHoldContinuous: {
		if(m_acc.isEmpty()) {
			m_acc = db;
			return m_acc;
		}
		const bool peak = mode == Averaging::PeakHoldContinuous;
		for(int i = 0; i < n; ++i) {
			m_acc[i] = peak ? std::max(m_acc[i], db[i]) : std::min(m_acc[i], db[i]);
		}
		return m_acc;
	}

	case Averaging::ExponentialRMS:
	case Averaging::ExponentialDB: {
		if(m_acc.isEmpty()) {
			m_acc = db;
			return m_acc;
		}
		// a = (x + (N-1)a) / N, on power or on dB.
		const double k = 1.0 / depth;
		for(int i = 0; i < n; ++i) {
			if(mode == Averaging::ExponentialRMS) {
				const double p = dbToPower(db[i]) * k + dbToPower(m_acc[i]) * (1.0 - k);
				m_acc[i] = powerToDb(p);
			} else {
				m_acc[i] = static_cast<float>(db[i] * k + m_acc[i] * (1.0 - k));
			}
		}
		return m_acc;
	}

	case Averaging::PeakHold:
	case Averaging::MinHold:
	case Averaging::LinearRMS:
	case Averaging::LinearDB:
		break;
	}

	m_frames.push_back(db);
	while(static_cast<int>(m_frames.size()) > depth) {
		m_frames.pop_front();
	}

	QVector<float> out(n);
	const double count = static_cast<double>(m_frames.size());
	for(int i = 0; i < n; ++i) {
		switch(mode) {
		case Averaging::PeakHold: {
			float v = m_frames.front()[i];
			for(const QVector<float> &f : m_frames) {
				v = std::max(v, f[i]);
			}
			out[i] = v;
			break;
		}
		case Averaging::MinHold: {
			float v = m_frames.front()[i];
			for(const QVector<float> &f : m_frames) {
				v = std::min(v, f[i]);
			}
			out[i] = v;
			break;
		}
		case Averaging::LinearRMS: {
			double sum = 0.0;
			for(const QVector<float> &f : m_frames) {
				sum += dbToPower(f[i]);
			}
			out[i] = powerToDb(sum / count);
			break;
		}
		default: { // LinearDB
			double sum = 0.0;
			for(const QVector<float> &f : m_frames) {
				sum += f[i];
			}
			out[i] = static_cast<float>(sum / count);
			break;
		}
		}
	}
	return out;
}

QVector<float> SpectrumProcessor::convert(const QVector<float> &db, Unit unit, double fullScale) const
{
	if(unit == Unit::DBFS) {
		return db;
	}

	// 0 dBFS is a full-scale sine: `fullScale` volts peak, fullScale/sqrt(2) volts RMS.
	const double vrmsFs = fullScale / M_SQRT2;
	QVector<float> out(db.size());

	if(unit == Unit::DBV || unit == Unit::DBU) {
		const double ref = unit == Unit::DBV ? 1.0 : kDbuRefVrms;
		const float offset = static_cast<float>(20.0 * std::log10(std::max(vrmsFs, 1e-30) / ref));
		for(int i = 0; i < db.size(); ++i) {
			out[i] = db[i] + offset;
		}
		return out;
	}

	double scale = unit == Unit::VPeak ? fullScale : vrmsFs;
	if(unit == Unit::VRmsPerSqrtHz && m_fft) {
		// Per-bin RMS over the noise bandwidth of one bin.
		const double fs = m_fft->sampleRate();
		const int nfft = m_fft->nfft();
		if(fs > 0.0 && nfft > 0) {
			scale /= std::sqrt(enbwBins(m_fft->window()) * fs / nfft);
		}
	}
	for(int i = 0; i < db.size(); ++i) {
		out[i] = static_cast<float>(scale * std::pow(10.0, db[i] / 20.0));
	}
	return out;
}

QWidget *SpectrumProcessor::createSettingsWidget(QWidget *parent)
{
	auto *section = new MenuSectionWidget();
	Style::setStyle(section, style::properties::widget::border);
	section->contentLayout()->setSpacing(10);

	auto *avgCombo = new MenuCombo(QStringLiteral("Averaging"), section);
	avgCombo->combo()->addItems(averagingNames());
	avgCombo->combo()->setCurrentIndex(static_cast<int>(averaging()));
	section->contentLayout()->addWidget(avgCombo);

	auto *depthSpin = new gui::MenuSpinbox(QStringLiteral("History"), history(), QStringLiteral("frames"), 1, 1000,
					       true, false, false, section);
	depthSpin->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	depthSpin->setVisible(usesHistory(averaging()));
	section->contentLayout()->addWidget(depthSpin);

	auto *unitCombo = new MenuCombo(QStringLiteral("Units"), section);
	unitCombo->combo()->addItems(unitNames());
	unitCombo->combo()->setCurrentIndex(static_cast<int>(unit()));
	section->contentLayout()->addWidget(unitCombo);

	connect(avgCombo->combo(), &QComboBox::currentIndexChanged, this,
		[this](int i) { setAveraging(static_cast<Averaging>(i)); });
	connect(this, &SpectrumProcessor::averagingChanged, section, [avgCombo, depthSpin](Averaging m) {
		QSignalBlocker b(avgCombo->combo());
		avgCombo->combo()->setCurrentIndex(static_cast<int>(m));
		depthSpin->setVisible(usesHistory(m));
	});
	connect(depthSpin, &gui::MenuSpinbox::valueChanged, this,
		[this](double v) { setHistory(static_cast<int>(v)); });
	connect(this, &SpectrumProcessor::historyChanged, depthSpin,
		[depthSpin](int n) { depthSpin->setValueSilent(n); });
	connect(unitCombo->combo(), &QComboBox::currentIndexChanged, this,
		[this](int i) { setUnit(static_cast<Unit>(i)); });
	connect(this, &SpectrumProcessor::unitChanged, unitCombo, [unitCombo](Unit u) {
		QSignalBlocker b(unitCombo->combo());
		unitCombo->combo()->setCurrentIndex(static_cast<int>(u));
	});

	return withBaseSettings(section, parent);
}

#include "moc_spectrumprocessor.cpp"
