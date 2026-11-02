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

#include <core/acq_engine/datastore.h>
#include <core/acq_engine/spectrumprocessor.h>

#include <QTest>

#include <cmath>

using namespace scopy::acq;

using Averaging = SpectrumProcessor::Averaging;
using Unit = SpectrumProcessor::Unit;

class TST_SpectrumProcessor : public QObject
{
	Q_OBJECT

private:
	const DataKey m_in = DataKey::withStage(QStringLiteral("fft"), QStringLiteral("ch"), QStringLiteral("db"));
	const DataKey m_out =
		DataKey::withStage(QStringLiteral("spec"), QStringLiteral("ch"), QStringLiteral("spectrum"));

	// Feeds `frames` through a fresh block, returning what it wrote for the last one.
	QVector<float> run(Averaging mode, int depth, const QList<QVector<float>> &frames, Unit unit = Unit::DBFS,
			   double fullScale = 1.0)
	{
		DataStore store;
		SpectrumProcessor proc(QStringLiteral("spec"), m_in, m_out);
		proc.setAveraging(mode);
		proc.setHistory(depth);
		proc.setUnit(unit);
		proc.setFullScaleVolts(fullScale);
		for(const QVector<float> &f : frames) {
			store.write(m_in, f);
			proc.process(&store);
		}
		return store.latestAs<QVector<float>>(m_out).value_or(QVector<float>{});
	}

	static void compare(const QVector<float> &got, const QVector<float> &want)
	{
		QCOMPARE(got.size(), want.size());
		for(int i = 0; i < got.size(); ++i) {
			QVERIFY2(std::abs(got[i] - want[i]) < 1e-3f,
				 qPrintable(QStringLiteral("bin %1: %2 != %3").arg(i).arg(got[i]).arg(want[i])));
		}
	}

private Q_SLOTS:
	void sampleIsPassthrough() { compare(run(Averaging::Sample, 5, {{-10, -20}, {-30, -40}}), {-30, -40}); }

	void peakHoldContinuous()
	{
		compare(run(Averaging::PeakHoldContinuous, 1, {{-10, -20}, {-5, -30}, {-50, -50}}), {-5, -20});
	}

	void minHoldContinuous()
	{
		compare(run(Averaging::MinHoldContinuous, 1, {{-10, -20}, {-5, -30}, {-8, -10}}), {-10, -30});
	}

	void peakHoldForgetsOldFrames()
	{
		// Depth 2: the first frame's -1 has left the window.
		compare(run(Averaging::PeakHold, 2, {{-1, -1}, {-10, -20}, {-5, -30}}), {-5, -20});
	}

	void minHoldWindow() { compare(run(Averaging::MinHold, 2, {{-90, -90}, {-10, -20}, {-5, -30}}), {-10, -30}); }

	void linearRmsAveragesPower()
	{
		// 0 dB and -10 dB are powers 1 and 0.1; their mean 0.55 is -2.596 dB.
		const float want = static_cast<float>(10.0 * std::log10(0.55));
		compare(run(Averaging::LinearRMS, 2, {{0}, {-10}}), {want});
	}

	void linearDbAveragesDecibels() { compare(run(Averaging::LinearDB, 2, {{-50}, {0}, {-10}}), {-5}); }

	void exponentialDb()
	{
		// a0 = -40; a1 = -20/4 + 3*(-40)/4 = -35.
		compare(run(Averaging::ExponentialDB, 4, {{-40}, {-20}}), {-35});
	}

	void exponentialRms()
	{
		// Powers 1 and 0.1 at depth 2: 0.55 again, from the recursive form.
		const float want = static_cast<float>(10.0 * std::log10(0.55));
		compare(run(Averaging::ExponentialRMS, 2, {{-10}, {0}}), {want});
	}

	void frameLengthChangeRestartsHistory()
	{
		compare(run(Averaging::PeakHoldContinuous, 1, {{0, 0}, {-10, -10, -10}}), {-10, -10, -10});
	}

	void modeChangeRestartsHistory()
	{
		DataStore store;
		SpectrumProcessor proc(QStringLiteral("spec"), m_in, m_out);
		proc.setAveraging(Averaging::PeakHoldContinuous);
		store.write(m_in, QVector<float>{0});
		proc.process(&store);
		proc.setAveraging(Averaging::MinHoldContinuous);
		store.write(m_in, QVector<float>{-10});
		proc.process(&store);
		compare(store.latestAs<QVector<float>>(m_out).value(), {-10});
	}

	void unitsFromFullScale()
	{
		// A 2 V peak full scale: 0 dBFS is 2 Vpeak, sqrt(2) Vrms, 3.0103 dBV.
		const double fs = 2.0;
		const float vrms = static_cast<float>(fs / M_SQRT2);
		compare(run(Averaging::Sample, 1, {{0}}, Unit::VPeak, fs), {2.0f});
		compare(run(Averaging::Sample, 1, {{0}}, Unit::VRms, fs), {vrms});
		compare(run(Averaging::Sample, 1, {{-20}}, Unit::VPeak, fs), {0.2f});
		compare(run(Averaging::Sample, 1, {{0}}, Unit::DBV, fs), {static_cast<float>(20.0 * std::log10(vrms))});
		compare(run(Averaging::Sample, 1, {{0}}, Unit::DBU, fs),
			{static_cast<float>(20.0 * std::log10(vrms / 0.7745966692))});
		compare(run(Averaging::Sample, 1, {{-7}}, Unit::DBFS, fs), {-7});
	}

	void streamInfoCarriesUnit()
	{
		SpectrumProcessor proc(QStringLiteral("spec"), m_in, m_out);
		proc.setUnit(Unit::DBV);
		const std::optional<StreamInfo> info = proc.streamInfo(m_out);
		QVERIFY(info.has_value());
		QCOMPARE(info->unit, QStringLiteral("dBV"));
		QCOMPARE(info->kind, ReprKind::Curve);
		QVERIFY(!proc.streamInfo(m_in).has_value());
	}
};

QTEST_MAIN(TST_SpectrumProcessor)
#include "tst_spectrumprocessor.moc"
