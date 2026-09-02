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

#include "acqaxis.h"
#include "acqchannel.h"
#include "acqplot.h"
#include "acqplotkind.h"
#include "acqplotmanager.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/DataStore.h>
#include <gui/instrumenttemplate.h>
#include <gui/plotchannel.h>
#include <gui/plotwidget.h>

#include <QApplication>
#include <QTest>

using namespace scopy;
using namespace scopy::adc;

// Deleting a channel goes through six objects that all hold pointers to each other —
// the rail row, the menu page, the plot's axis pool, the PlotWidget's selection, the
// navigator/tracker maps and the measurement panels. The last channel is the case
// where every one of those transitions to empty at once, which is the case a live
// session crashed on.
class TST_AcqChannelDelete : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void deleteLastChannelOnBasicPlot();
	void deleteLastChannelOnWaterfallPlot();
	void deleteEveryPlot();
	void deletePlotWithViewOptionsOn();
	void sampleIndexIsMonotonicAcrossChunks();
	void sampleIndexSpansPlotWindowWhileFilling();
	void retargetToSampleIndexResetsInterval();

private:
	// A store with two written keys, so channels created against them are key-present
	// and build their real visuals rather than the greyed-out path.
	static void writeKeys(scopy::acq::DataStore *store);
};

void TST_AcqChannelDelete::writeKeys(scopy::acq::DataStore *store)
{
	const QVector<float> data(64, 1.0f);
	store->write(scopy::acq::DataKey::raw("dev", "voltage0"), data);
	store->write(scopy::acq::DataKey::raw("dev", "voltage1"), data);
}

void TST_AcqChannelDelete::deleteLastChannelOnBasicPlot()
{
	scopy::acq::DataStore store;
	writeKeys(&store);

	InstrumentTemplate shell;
	// Engine-less: these cases only exercise channel and plot teardown, and a null
	// engine simply means every descriptor resolves to a default-built StreamInfo.
	AcqPlotManager     mgr(&store, nullptr, &shell);

	AcqPlot *p = mgr.addPlot("P", AcqPlotKind::Basic);
	QVERIFY(p);

	AcqChannel *a = mgr.addChannel(p, scopy::acq::ReprKind::Curve, scopy::acq::DataKey::raw("dev", "voltage0"));
	AcqChannel *b = mgr.addChannel(p, scopy::acq::ReprKind::Curve, scopy::acq::DataKey::raw("dev", "voltage1"));
	QVERIFY(a && b);
	QCOMPARE(mgr.channels().size(), 2);

	// Second-to-last first: this is the path the user reported as working, so a crash
	// here would mean the bug is not about emptiness at all.
	mgr.removeChannel(a);
	QCOMPARE(mgr.channels().size(), 1);
	mgr.replot();
	QCoreApplication::processEvents();

	// The reported crash.
	mgr.removeChannel(b);
	QCOMPARE(mgr.channels().size(), 0);
	// Both of these ran after the crash in the live session: the frame timer's replot
	// and the deleteLater the removal queued.
	mgr.replot();
	QCoreApplication::processEvents();
	mgr.replot();
}

void TST_AcqChannelDelete::deleteLastChannelOnWaterfallPlot()
{
	scopy::acq::DataStore store;
	writeKeys(&store);

	InstrumentTemplate shell;
	// Engine-less: these cases only exercise channel and plot teardown, and a null
	// engine simply means every descriptor resolves to a default-built StreamInfo.
	AcqPlotManager     mgr(&store, nullptr, &shell);

	// A distinct path: this kind declines both pooled axes and wraps the widget's own
	// pair, so releaseAxes() takes a different branch than the curve above.
	AcqPlot *p = mgr.addPlot("W", AcqPlotKind::Waterfall);
	QVERIFY(p);
	AcqChannel *ch =
		mgr.addChannel(p, scopy::acq::ReprKind::Waterfall, scopy::acq::DataKey::raw("dev", "voltage0"));
	QVERIFY(ch);

	mgr.removeChannel(ch);
	QCOMPARE(mgr.channels().size(), 0);
	mgr.replot();
	QCoreApplication::processEvents();
}

void TST_AcqChannelDelete::deleteEveryPlot()
{
	scopy::acq::DataStore store;
	writeKeys(&store);

	InstrumentTemplate shell;
	// Engine-less: these cases only exercise channel and plot teardown, and a null
	// engine simply means every descriptor resolves to a default-built StreamInfo.
	AcqPlotManager     mgr(&store, nullptr, &shell);

	AcqPlot *p1 = mgr.addPlot("P1", AcqPlotKind::Basic);
	AcqPlot *p2 = mgr.addPlot("P2", AcqPlotKind::Basic);
	QVERIFY(p1 && p2);
	mgr.addChannel(p1, scopy::acq::ReprKind::Curve, scopy::acq::DataKey::raw("dev", "voltage0"));
	mgr.addChannel(p2, scopy::acq::ReprKind::Curve, scopy::acq::DataKey::raw("dev", "voltage1"));

	// Plot removal deletes the plot's channels, so this covers the same last-channel
	// transition reached from the other direction. An empty manager is a valid state.
	mgr.removePlot(p1->uuid());
	QCoreApplication::processEvents();
	mgr.removePlot(p2->uuid());
	QCoreApplication::processEvents();
	QCOMPARE(mgr.plots().size(), 0);
	QCOMPARE(mgr.channels().size(), 0);
	mgr.replot();
}

void TST_AcqChannelDelete::deletePlotWithViewOptionsOn()
{
	scopy::acq::DataStore store;
	writeKeys(&store);

	InstrumentTemplate shell;
	// Engine-less: these cases only exercise channel and plot teardown, and a null
	// engine simply means every descriptor resolves to a default-built StreamInfo.
	AcqPlotManager     mgr(&store, nullptr, &shell);

	AcqPlot *p = mgr.addPlot("V", AcqPlotKind::Basic);
	QVERIFY(p);
	AcqChannel *ch = mgr.addChannel(p, scopy::acq::ReprKind::Curve, scopy::acq::DataKey::raw("dev", "voltage0"));
	QVERIFY(ch);

	// The three per-plot view options are all built lazily on first enable, and two of
	// them install children on the plot widget's canvas — a legend overlay and the
	// cursors' four draggable handles. Turning them on before the delete is what puts
	// those children in the teardown path at all.
	p->setShowLegend(true);
	p->setShowCursors(true);
	QVERIFY(p->cursorSettings(&shell));
	p->setShowLabels(false);
	mgr.replot();
	QCoreApplication::processEvents();

	// Labels off must survive a selection: PlotWidget::selectChannel() calls
	// showAxisLabels() itself, so without AcqPlot re-asserting the flag this would put
	// back exactly what was hidden.
	if(PlotChannel *pc = ch->plotChannel()) {
		p->plot()->selectChannel(pc);
		QCoreApplication::processEvents();
		QVERIFY(!p->showLabels());
	}

	// Channel first, then the plot: the legend follows removedChannel while the cursors
	// stay bound to a canvas that is about to go.
	mgr.removeChannel(ch);
	QCoreApplication::processEvents();
	mgr.replot();

	mgr.removePlot(p->uuid());
	QCoreApplication::processEvents();
	QCOMPARE(mgr.plots().size(), 0);
	mgr.replot();
}

// The reason the ramp is one chunk written once rather than per cycle.
//
// SampleBuffer::window() stitches chunks oldest-first to fill a window wider than one
// buffer, so a ramp written per acquisition cycle reads back as 0..bufferSize-1 repeated
// — a sawtooth — and every curve on it folds over itself once per chunk. A single chunk
// at least plotSize long has no stitching to do: the window is one mid() of it.
void TST_AcqChannelDelete::sampleIndexIsMonotonicAcrossChunks()
{
	constexpr int kBuffer = 64;
	constexpr int kPlotSize = 4 * kBuffer;

	scopy::acq::DataStore store;
	scopy::acq::AcquisitionEngine engine(&store);

	InstrumentTemplate shell;
	// Engine-less: these cases only exercise channel and plot teardown, and a null
	// engine simply means every descriptor resolves to a default-built StreamInfo.
	AcqPlotManager     mgr(&store, nullptr, &shell);
	// The connection AcqInstrumentController makes, so the ramp tracks the width the
	// plots actually ask for rather than the engine's default.
	QObject::connect(&mgr, &AcqPlotManager::maxWindowSizeChanged, &engine,
			 [&engine](int n) { engine.setIndexRampLength(static_cast<std::size_t>(n)); });
	mgr.setPlotSize(kPlotSize);

	AcqPlot *p = mgr.addPlot("M", AcqPlotKind::Basic);
	QVERIFY(p);
	const scopy::acq::DataKey y = scopy::acq::DataKey::raw("dev", "voltage0");
	// Written before the channel exists, or addChannel() sees an absent key, marks the
	// channel key-absent and switches it off — and a disabled channel reads nothing.
	// keysChanged is a queued connection, so nothing would re-enable it inside this slot.
	store.write(y, QVector<float>(kBuffer, 0.0f));
	AcqChannel *ch = mgr.addChannel(p, scopy::acq::ReprKind::Curve, y);
	QVERIFY(ch);
	QVERIFY(ch->plotChannel());
	// Resolved to the engine's ramp, not left empty: that resolution is what lets every
	// reader treat X as an ordinary stream.
	QCOMPARE(ch->xKey(), scopy::acq::AcquisitionEngine::indexRampKey());

	// Four buffers' worth, one chunk at a time, exactly as a source block writes.
	for(int c = 0; c < 4; ++c) {
		QVector<float> chunk(kBuffer);
		for(int i = 0; i < kBuffer; ++i) {
			chunk[i] = static_cast<float>(c * kBuffer + i);
		}
		store.beginCycle();
		store.write(y, chunk);
		mgr.onCycleComplete();
	}

	QwtPlotCurve *curve = ch->plotChannel()->curve();
	QVERIFY(curve);
	// The full window, stitched from four chunks — this is the case a per-cycle ramp got
	// wrong.
	QCOMPARE(static_cast<int>(curve->dataSize()), kPlotSize);
	for(size_t i = 1; i < curve->dataSize(); ++i) {
		const double prev = curve->sample(i - 1).x();
		const double cur = curve->sample(i).x();
		QVERIFY2(cur > prev,
			 qPrintable(QStringLiteral("X went backwards at %1: %2 then %3")
					    .arg(i)
					    .arg(prev)
					    .arg(cur)));
	}
	// Window-relative, so the axis reads the same whatever the ramp's absolute offset.
	QCOMPARE(curve->sample(0).x(), 0.0);
	QCOMPARE(curve->sample(curve->dataSize() - 1).x(), double(kPlotSize - 1));

	// And time is that ramp over the rate: same window, seconds.
	mgr.setFallbackSampleRate(1000.0);
	QVERIFY(ch->xAxis());
	ch->xAxis()->setSource(AcqAxis::Source::time());
	QVERIFY(ch->xAxis()->isTime());
	// Refilled after the retarget: a source change re-registers the channel's claims from
	// scratch, and the release side trims what the old claim was holding — so the Y history
	// is back to one chunk until four more cycles have run. That is the store's behaviour on
	// any retarget and predates the ramp; what is under test here is only the X values.
	for(int c = 4; c < 8; ++c) {
		QVector<float> chunk(kBuffer);
		for(int i = 0; i < kBuffer; ++i) {
			chunk[i] = static_cast<float>(c * kBuffer + i);
		}
		store.beginCycle();
		store.write(y, chunk);
		mgr.onCycleComplete();
	}
	QCOMPARE(static_cast<int>(curve->dataSize()), kPlotSize);
	QCOMPARE(curve->sample(0).x(), 0.0);
	// Tolerance, not qFuzzyCompare: the seconds vector is Float32 (setSamples takes float*),
	// so 0.255 arrives as 0.25499999523... — exact to float, nowhere near double-fuzzy.
	const double last = curve->sample(curve->dataSize() - 1).x();
	const double want = (kPlotSize - 1) / 1000.0;
	QVERIFY2(qAbs(last - want) < 1e-6, qPrintable(QStringLiteral("%1 vs %2").arg(last).arg(want)));
}

// The sample index is the position in the plot window, not a count of what has arrived.
//
// While history fills, the X window comes back at full width and Y does not, so the newest
// samples are the ones that pair up. Two things used to be taken from the data instead of
// the window: the values (the *trimmed* tail was rebased to 0, so a half-full 256-wide plot
// read 0..127) and the axis range (set to the extent of what was drawn). Either one alone
// makes the curve fill the whole width and the axis rescale on every cycle. Now the values
// are rebased from the start of the window and the range is pinned to the window, so a
// half-full plot draws at 128..255 on a motionless 0..255 axis.
void TST_AcqChannelDelete::sampleIndexSpansPlotWindowWhileFilling()
{
	constexpr int kBuffer = 64;
	constexpr int kPlotSize = 4 * kBuffer;

	scopy::acq::DataStore store;
	scopy::acq::AcquisitionEngine engine(&store);

	InstrumentTemplate shell;
	AcqPlotManager     mgr(&store, nullptr, &shell);
	QObject::connect(&mgr, &AcqPlotManager::maxWindowSizeChanged, &engine,
			 [&engine](int n) { engine.setIndexRampLength(static_cast<std::size_t>(n)); });
	mgr.setPlotSize(kPlotSize);

	AcqPlot *p = mgr.addPlot("F", AcqPlotKind::Basic);
	QVERIFY(p);
	const scopy::acq::DataKey y = scopy::acq::DataKey::raw("dev", "voltage0");
	// Chunk 0, written before the channel exists so addChannel() sees the key present —
	// same reason as in sampleIndexIsMonotonicAcrossChunks(). It is a real chunk of history,
	// so the cycle below leaves the window half full, not quarter full.
	store.write(y, QVector<float>(kBuffer, 0.0f));
	AcqChannel *ch = mgr.addChannel(p, scopy::acq::ReprKind::Curve, y);
	QVERIFY(ch);
	QVERIFY(ch->plotChannel());
	QVERIFY(ch->xAxis());

	QVector<float> chunk(kBuffer);
	const auto pushChunk = [&](int c) {
		for(int i = 0; i < kBuffer; ++i) {
			chunk[i] = static_cast<float>(c * kBuffer + i);
		}
		store.beginCycle();
		store.write(y, chunk);
		mgr.onCycleComplete();
	};

	// Two buffers of a four-buffer window.
	pushChunk(1);
	constexpr int kFilled = 2 * kBuffer;

	QwtPlotCurve *curve = ch->plotChannel()->curve();
	QVERIFY(curve);
	QCOMPARE(static_cast<int>(curve->dataSize()), kFilled);
	// Flush right: the half of the window that has data occupies its second half.
	QCOMPARE(curve->sample(0).x(), double(kPlotSize - kFilled));
	QCOMPARE(curve->sample(curve->dataSize() - 1).x(), double(kPlotSize - 1));
	// And the axis spans the whole window regardless — the left half is simply empty. The
	// minimum is the assertion that matters: with the range taken from the data it read
	// kPlotSize-kFilled here, so the curve filled the full width and the axis rescaled on
	// every cycle until history caught up.
	QCOMPARE(ch->xAxis()->min(), 0.0);
	QCOMPARE(ch->xAxis()->max(), double(kPlotSize - 1));

	// The rest of the window, which must not move the axis — that is the whole point.
	pushChunk(2);
	pushChunk(3);
	QCOMPARE(static_cast<int>(curve->dataSize()), kPlotSize);
	QCOMPARE(curve->sample(0).x(), 0.0);
	QCOMPARE(curve->sample(curve->dataSize() - 1).x(), double(kPlotSize - 1));
	QCOMPARE(ch->xAxis()->min(), 0.0);
	QCOMPARE(ch->xAxis()->max(), double(kPlotSize - 1));
}

// Retargeting an X axis *back* to sample index used to leave the previous stream's range
// on it: the only two setInterval(0, plotSize-1) calls ran on channel add and on a width
// change, so switching an FFT channel's X from Hz to sample index kept the Hz range until
// the reader touched the plot-width spinbox. Now the sample index is a stream and the
// channel's own requestInterval() covers it on the same path as any other X.
void TST_AcqChannelDelete::retargetToSampleIndexResetsInterval()
{
	constexpr int kPlotSize = 64;

	scopy::acq::DataStore store;
	scopy::acq::AcquisitionEngine engine(&store);
	engine.setIndexRampLength(kPlotSize);

	InstrumentTemplate shell;
	// Engine-less: these cases only exercise channel and plot teardown, and a null
	// engine simply means every descriptor resolves to a default-built StreamInfo.
	AcqPlotManager     mgr(&store, nullptr, &shell);
	mgr.setPlotSize(kPlotSize);
	engine.setIndexRampLength(kPlotSize);

	// A Y stream and an X stream on a range nothing like 0..plotSize-1, so a stale
	// interval is unmistakable.
	const scopy::acq::DataKey y = scopy::acq::DataKey::raw("dev", "mag");
	const scopy::acq::DataKey f = scopy::acq::DataKey::raw("dev", "freq");
	QVector<float> mag(kPlotSize, 1.0f), freq(kPlotSize);
	for(int i = 0; i < kPlotSize; ++i) {
		freq[i] = 1.0e6f + 1.0e3f * i;
	}
	store.beginCycle();
	store.write(y, mag);
	store.write(f, freq);

	AcqPlot *p = mgr.addPlot("R", AcqPlotKind::Basic);
	QVERIFY(p);
	AcqChannel *ch = mgr.addChannel(p, scopy::acq::ReprKind::Curve, y, f);
	QVERIFY(ch);
	QVERIFY(ch->xAxis());

	mgr.onCycleComplete();
	QVERIFY2(ch->xAxis()->min() > 1.0e5, qPrintable(QString::number(ch->xAxis()->min())));

	// The retarget, and then a read — with no setPlotSize() in between, which is the
	// whole point.
	ch->xAxis()->setSource(AcqAxis::Source::sampleIndex());
	mgr.onCycleComplete();

	QCOMPARE(ch->xAxis()->min(), 0.0);
	QCOMPARE(ch->xAxis()->max(), double(kPlotSize - 1));
}

QTEST_MAIN(TST_AcqChannelDelete)

#include "tst_acqchanneldelete.moc"
