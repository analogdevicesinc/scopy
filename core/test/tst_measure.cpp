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

#include <core/acq_engine/datastore.h>
#include <core/acq_engine/measure.h>
#include <core/acq_engine/measureprocessor.h>

#include <QSignalSpy>

#include <QTest>
#include <QVector>

#include <cmath>

using namespace scopy::acq;

class TST_Measure : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void sine();
	void square();
	void dutyCycle();
	void gate();
	void flatSignalHasNoPeriod();
	void namesAndUnits();
	void processorKeepsStatistics();

private:
	static QVector<float> sine(int n, double periodSamples, double amplitude, double offset);
	static QVector<float> square(int n, int periodSamples, int highSamples, float low, float high);
};

QVector<float> TST_Measure::sine(int n, double periodSamples, double amplitude, double offset)
{
	QVector<float> out(n);
	for(int i = 0; i < n; ++i) {
		out[i] = offset + amplitude * std::sin(2 * M_PI * i / periodSamples);
	}
	return out;
}

QVector<float> TST_Measure::square(int n, int periodSamples, int highSamples, float low, float high)
{
	QVector<float> out(n);
	for(int i = 0; i < n; ++i) {
		out[i] = (i % periodSamples) < highSamples ? high : low;
	}
	return out;
}

void TST_Measure::sine()
{
	// 1 kHz at 1 MHz, 2 V amplitude around 0.5 V.
	const QVector<float> d = sine(10000, 1000, 2.0, 0.5);
	MeasureParams p;
	p.sampleRate = 1e6;
	const MeasureResult r = measure(d.constData(), d.size(), p);
	QVERIFY(std::abs(r.value(Measure::Period) - 1e-3) < 1e-6);
	QVERIFY(std::abs(r.value(Measure::Frequency) - 1e3) < 1.0);
	QVERIFY(std::abs(r.value(Measure::Max) - 2.5) < 1e-3);
	QVERIFY(std::abs(r.value(Measure::Min) + 1.5) < 1e-3);
	QVERIFY(std::abs(r.value(Measure::PeakPeak) - 4.0) < 1e-3);
	QVERIFY(std::abs(r.value(Measure::Mean) - 0.5) < 1e-3);
	QVERIFY(std::abs(r.value(Measure::AcRms) - 2.0 / std::sqrt(2.0)) < 1e-3);
	QVERIFY(std::abs(r.value(Measure::Rms) - std::sqrt(0.25 + 2.0)) < 1e-3);
	QVERIFY(std::abs(r.value(Measure::CycleMean) - 0.5) < 1e-2);
	QVERIFY(std::abs(r.value(Measure::PosDuty) - 50.0) < 0.5);
	// 10 % to 90 % of a sine is 0.295 of a period.
	const double rise = std::asin(0.8) / M_PI;
	QVERIFY(std::abs(r.value(Measure::Rise) - rise * 1e-3) < 5e-6);
	QVERIFY(std::abs(r.value(Measure::Fall) - rise * 1e-3) < 5e-6);
	QVERIFY(r.marks.periodStart >= 0);
	QCOMPARE(r.marks.periodEnd - r.marks.periodStart, 1000);
}

void TST_Measure::square()
{
	const QVector<float> d = square(5000, 100, 50, -1.0f, 3.0f);
	MeasureParams p;
	p.sampleRate = 1e3;
	const MeasureResult r = measure(d.constData(), d.size(), p);
	QCOMPARE(r.value(Measure::Period), 0.1);
	QCOMPARE(r.value(Measure::Low), -1.0);
	QCOMPARE(r.value(Measure::High), 3.0);
	QCOMPARE(r.value(Measure::Amplitude), 4.0);
	QCOMPARE(r.value(Measure::Middle), 1.0);
	QCOMPARE(r.value(Measure::PosOver), 0.0);
	QCOMPARE(r.value(Measure::PosWidth), 0.05);
	QCOMPARE(r.value(Measure::NegWidth), 0.05);
}

void TST_Measure::dutyCycle()
{
	const QVector<float> d = square(10000, 200, 50, 0.0f, 5.0f);
	MeasureParams p;
	p.sampleRate = 1e6;
	const MeasureResult r = measure(d.constData(), d.size(), p);
	QVERIFY(std::abs(r.value(Measure::PosDuty) - 25.0) < 1.0);
	QVERIFY(std::abs(r.value(Measure::NegDuty) - 75.0) < 1.0);
	QVERIFY(std::abs(r.value(Measure::Period) - 200e-6) < 1e-9);
}

void TST_Measure::gate()
{
	// A square, then a flat line: gated to the flat part there is nothing periodic.
	QVector<float> d = square(2000, 100, 50, 0.0f, 1.0f);
	d.append(QVector<float>(2000, 0.25f));
	MeasureParams p;
	p.sampleRate = 1e3;
	p.gateStart = 2000;
	MeasureResult r = measure(d.constData(), d.size(), p);
	QCOMPARE(r.value(Measure::Mean), 0.25);
	QVERIFY(std::isnan(r.value(Measure::Period)));
	p.gateStart = 0;
	p.gateEnd = 2000;
	r = measure(d.constData(), d.size(), p);
	QCOMPARE(r.value(Measure::Period), 0.1);
}

void TST_Measure::flatSignalHasNoPeriod()
{
	const QVector<float> d(1000, 1.0f);
	const MeasureResult r = measure(d.constData(), d.size(), MeasureParams{});
	QCOMPARE(r.value(Measure::Mean), 1.0);
	QVERIFY(std::isnan(r.value(Measure::Period)));
	QVERIFY(std::isnan(r.value(Measure::Rise)));
	QVERIFY(std::isnan(measure(nullptr, 0, MeasureParams{}).value(Measure::Mean)));
}

void TST_Measure::namesAndUnits()
{
	QCOMPARE(measureId(Measure::PeakPeak), QStringLiteral("peak_peak"));
	QCOMPARE(measureUnit(Measure::Mean), QStringLiteral("V"));
	QCOMPARE(measureUnit(Measure::Area), QStringLiteral("Vs"));
	QCOMPARE(measureUnit(Measure::Frequency), QStringLiteral("Hz"));
	QCOMPARE(measureUnit(Measure::PosDuty), QStringLiteral("%"));
	QCOMPARE(measureName(Measure::NegDuty), QStringLiteral("-Duty"));
}

void TST_Measure::processorKeepsStatistics()
{
	const DataKey key = DataKey::raw(QStringLiteral("osc"), QStringLiteral("ch1"));
	DataStore store;
	MeasureProcessor proc;
	proc.setWatchedKeys({key});
	proc.setSampleRate(1e3);
	proc.setEmitInterval(0);
	QSignalSpy spy(&proc, &MeasureProcessor::measured);
	store.write(key, QVector<float>(100, 1.0f));
	proc.process(&store);
	store.write(key, QVector<float>(100, 3.0f));
	proc.process(&store);
	QCOMPARE(spy.count(), 2);
	MeasureSnapshot snap = spy.last().first().value<MeasureSnapshot>();
	QCOMPARE(snap.key, key);
	const MeasureStat &mean = snap.stats[static_cast<int>(Measure::Mean)];
	QCOMPARE(mean.count, 2);
	QCOMPARE(mean.average, 2.0);
	QCOMPARE(mean.min, 1.0);
	QCOMPARE(mean.max, 3.0);

	proc.clearStats();
	proc.process(&store);
	snap = spy.last().first().value<MeasureSnapshot>();
	QCOMPARE(snap.stats[static_cast<int>(Measure::Mean)].count, 1);
}

QTEST_MAIN(TST_Measure)
#include "tst_measure.moc"
