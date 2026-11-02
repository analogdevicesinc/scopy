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

#pragma once

#include "scopy-core_export.h"

#include "datakey.h"
#include "processorblock.h"

#include <deque>
#include <QMutex>
#include <QPointer>
#include <QStringList>
#include <QVector>

namespace scopy {
namespace acq {

class GenalyzerFFTProcessor;

// Averages or holds a GenalyzerFFTProcessor's dBFS frames across cycles and converts them
// to the selected unit. One block per channel: the engine runs a processor only once all of
// its inputs were written that cycle.
class SCOPY_CORE_EXPORT SpectrumProcessor : public ProcessorBlock
{
	Q_OBJECT
public:
	enum class Averaging
	{
		Sample,
		PeakHoldContinuous,
		PeakHold,
		MinHoldContinuous,
		MinHold,
		LinearRMS,
		LinearDB,
		ExponentialRMS,
		ExponentialDB,
	};

	enum class Unit
	{
		DBFS,
		DBV,
		DBU,
		VPeak,
		VRms,
		VRmsPerSqrtHz,
	};

	SpectrumProcessor(const QString &name, const DataKey &inputKey, const DataKey &outputKey,
			  const GenalyzerFFTProcessor *fft = nullptr, QObject *parent = nullptr);

	DataKey inputKey() const { return m_inputKey; }
	DataKey outputKey() const { return m_outputKey; }

	void setAveraging(Averaging mode);
	Averaging averaging() const;

	void setHistory(int frames);
	int history() const;

	void setUnit(Unit unit);
	Unit unit() const;

	void setFullScaleVolts(double volts);
	double fullScaleVolts() const;

	static QStringList averagingNames();
	static QStringList unitNames();
	static QString unitSymbol(Unit unit);
	static bool usesHistory(Averaging mode);
	static double enbwBins(int gnWindow);

	QList<DataKey> outputKeys() const override { return {m_outputKey}; }
	std::optional<StreamInfo> streamInfo(const DataKey &key) const override;

	void process(DataStore *store) override;
	void reset() override;
	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

Q_SIGNALS:
	void averagingChanged(scopy::acq::SpectrumProcessor::Averaging mode);
	void historyChanged(int frames);
	void unitChanged(scopy::acq::SpectrumProcessor::Unit unit);

private:
	QVector<float> average(const QVector<float> &db, Averaging mode, int depth);
	QVector<float> convert(const QVector<float> &db, Unit unit, double fullScale) const;
	void clearHistoryLocked();

	DataKey m_inputKey;
	DataKey m_outputKey;
	QPointer<const GenalyzerFFTProcessor> m_fft;

	mutable QMutex m_mutex;
	Averaging m_averaging{Averaging::Sample};
	int m_history{10};
	Unit m_unit{Unit::DBFS};
	double m_fullScaleVolts{1.0};
	bool m_historyStale{true};

	std::deque<QVector<float>> m_frames;
	QVector<float> m_acc;
};

} // namespace acq
} // namespace scopy

Q_DECLARE_METATYPE(scopy::acq::SpectrumProcessor::Averaging)
Q_DECLARE_METATYPE(scopy::acq::SpectrumProcessor::Unit)
