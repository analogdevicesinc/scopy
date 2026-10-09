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

#include <gui/waterfallplotwidget.h>

#include <QTest>

#include <cfloat>

using namespace scopy;

// The waterfall's ring buffer. WaterfallData is the whole per-frame cost of the
// waterfall — appendRow() runs once per new chunk and value() once per raster pixel
// per repaint — so what is pinned here is the index arithmetic that makes both cheap:
// the mapping from a logical row to a ring slot, and its behaviour across the three
// events that move the ring's geometry (a row-count change, a bin-count change, and a
// bulk snapshot fill).
//
// The ring is private and there is no row accessor, so rows are read back through
// value() — the same path Qwt uses, which makes these assertions about what actually
// gets drawn rather than about the container. Antialiasing is off throughout so a tap
// is one sample rather than a bilinear blend of four.
class TST_WaterfallData : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void appendOrdersNewestLast();
	void appendWrapsPastMaxRows();
	void appendBeforeMaxRowsIsDropped();
	void binCountChangeClearsHistory();
	void narrowerRowIsAlsoABinCountChange();
	void growMaxRowsKeepsRows();
	void shrinkMaxRowsKeepsNewest();
	void shrinkMaxRowsWhileWrapped();
	void resetKeepsGeometry();
	void snapshotMatchesEquivalentAppends();
	void snapshotDropsRowsBeyondMaxRows();
	void snapshotAcceptsRaggedRows();
	void valueRejectsOutOfRange();
};

namespace {

constexpr int kBins = 8;

// The x that lands exactly on bin b, inverting value()'s
// binF = (x - min) / range * (bins - 1) for the unit interval.
double xOfBin(int b, int bins = kBins) { return static_cast<double>(b) / static_cast<double>(bins - 1); }

// The y that lands exactly on logical row r (0 = oldest live row). value() maps
// dataRowF = (maxRows - 1 - y) - (maxRows - nRows), so y = nRows - 1 - r: a
// partially-filled history is anchored to the bottom of the axis.
double yOfRow(int r, int liveRows) { return static_cast<double>(liveRows - 1 - r); }

// One sample, as Qwt would sample it.
double at(const WaterfallData &d, int r, int b, int bins = kBins)
{
	return d.value(xOfBin(b, bins), yOfRow(r, d.rowCount()));
}

// A row whose every bin is derived from `id`, so a row can be identified from any
// single bin and a misordering cannot be mistaken for a correct one.
std::vector<float> mkRow(int id, int bins = kBins)
{
	std::vector<float> r(static_cast<size_t>(bins));
	for(int b = 0; b < bins; ++b)
		r[static_cast<size_t>(b)] = static_cast<float>(id * 100 + b);
	return r;
}

// A WaterfallData with a known geometry and no interpolation, which is what makes
// at() a plain sample read.
void setup(WaterfallData &d, int maxRows, double xMax = 1.0)
{
	d.setMaxRows(maxRows);
	d.setXInterval(0.0, xMax);
	// Wide enough that nothing under test is clipped to the range's ends.
	d.setZInterval(-1e9, 1e9);
	d.setAntialiasing(false);
}

void append(WaterfallData &d, int id, int bins = kBins)
{
	const std::vector<float> r = mkRow(id, bins);
	d.appendRow(r.data(), r.size());
}

} // namespace

void TST_WaterfallData::appendOrdersNewestLast()
{
	WaterfallData d;
	setup(d, 4);
	append(d, 1);
	append(d, 2);
	append(d, 3);

	QCOMPARE(d.rowCount(), 3);
	// r = 0 is the oldest, so the first row appended must read back at r = 0.
	QCOMPARE(at(d, 0, 0), 100.0);
	QCOMPARE(at(d, 1, 0), 200.0);
	QCOMPARE(at(d, 2, 0), 300.0);
	// Every bin, not just bin 0: this is what catches a row copied at the wrong
	// offset into the ring rather than merely in the wrong order.
	for(int b = 0; b < kBins; ++b)
		QCOMPARE(at(d, 2, b), 300.0 + b);
}

void TST_WaterfallData::appendWrapsPastMaxRows()
{
	WaterfallData d;
	setup(d, 3);
	for(int i = 1; i <= 5; ++i)
		append(d, i);

	// Saturated, not grown: the ring is preallocated and the oldest rows are the
	// ones overwritten.
	QCOMPARE(d.rowCount(), 3);
	QCOMPARE(d.maxRows(), 3);
	QCOMPARE(at(d, 0, 0), 300.0);
	QCOMPARE(at(d, 1, 0), 400.0);
	QCOMPARE(at(d, 2, 0), 500.0);
}

void TST_WaterfallData::appendBeforeMaxRowsIsDropped()
{
	WaterfallData d;
	d.setXInterval(0.0, 1.0);
	d.setAntialiasing(false);
	// m_maxRows is 0 until setMaxRows() runs, so there is nowhere to put the row.
	append(d, 1);
	QCOMPARE(d.rowCount(), 0);
	QCOMPARE(d.value(0.0, 0.0), -DBL_MAX);

	// The dropped row must not have latched the bin count on its way out: if it had,
	// value() would index a ring that was never allocated.
	setup(d, 2);
	append(d, 7);
	QCOMPARE(d.rowCount(), 1);
	QCOMPARE(at(d, 0, 0), 700.0);
}

void TST_WaterfallData::binCountChangeClearsHistory()
{
	WaterfallData d;
	setup(d, 4);
	append(d, 1);
	append(d, 2);
	QCOMPARE(d.rowCount(), 2);

	// Stored rows are spectra of the old width, so they are not comparable with the
	// new ones and are dropped rather than padded.
	const std::vector<float> wide = mkRow(3, kBins * 2);
	d.appendRow(wide.data(), wide.size());
	QCOMPARE(d.rowCount(), 1);
	QCOMPARE(at(d, 0, 0, kBins * 2), 300.0);
	QCOMPARE(at(d, 0, kBins * 2 - 1, kBins * 2), 300.0 + kBins * 2 - 1);
}

void TST_WaterfallData::narrowerRowIsAlsoABinCountChange()
{
	// Narrowing is a width change too, not a short row to pad: a different bin count
	// is a different spectrum in both directions. Padding is setSnapshot's behaviour
	// for a ragged snapshot, which is a separate case — see snapshotAcceptsRaggedRows.
	WaterfallData d;
	setup(d, 4);
	append(d, 1);
	const std::vector<float> narrow = mkRow(2, 4);
	d.appendRow(narrow.data(), narrow.size());
	QCOMPARE(d.rowCount(), 1);
	QCOMPARE(at(d, 0, 0, 4), 200.0);
}

void TST_WaterfallData::growMaxRowsKeepsRows()
{
	WaterfallData d;
	setup(d, 3);
	append(d, 1);
	append(d, 2);

	d.setMaxRows(8);
	QCOMPARE(d.maxRows(), 8);
	QCOMPARE(d.rowCount(), 2);
	QCOMPARE(at(d, 0, 0), 100.0);
	QCOMPARE(at(d, 1, 0), 200.0);

	// And the ring is usable afterwards at the new size rather than only readable.
	append(d, 3);
	QCOMPARE(d.rowCount(), 3);
	QCOMPARE(at(d, 2, 0), 300.0);
}

void TST_WaterfallData::shrinkMaxRowsKeepsNewest()
{
	WaterfallData d;
	setup(d, 6);
	for(int i = 1; i <= 5; ++i)
		append(d, i);

	d.setMaxRows(2);
	// The newest two, not the first two: shrinking history drops the oldest end,
	// which is what the row spinbox means.
	QCOMPARE(d.rowCount(), 2);
	QCOMPARE(at(d, 0, 0), 400.0);
	QCOMPARE(at(d, 1, 0), 500.0);

	append(d, 6);
	QCOMPARE(d.rowCount(), 2);
	QCOMPARE(at(d, 0, 0), 500.0);
	QCOMPARE(at(d, 1, 0), 600.0);
}

void TST_WaterfallData::shrinkMaxRowsWhileWrapped()
{
	// The case that catches a rebuild wrapping the old block modulo the *new* row
	// count: the ring must already be wrapped when the geometry changes, so the
	// oldest live row is not at slot 0.
	WaterfallData d;
	setup(d, 4);
	for(int i = 1; i <= 6; ++i) // head has wrapped twice
		append(d, i);
	QCOMPARE(at(d, 0, 0), 300.0);

	d.setMaxRows(3);
	QCOMPARE(d.rowCount(), 3);
	QCOMPARE(at(d, 0, 0), 400.0);
	QCOMPARE(at(d, 1, 0), 500.0);
	QCOMPARE(at(d, 2, 0), 600.0);

	// Grow back from the wrapped-then-rebuilt state, then keep appending: a rebuild
	// that left m_head inconsistent with m_count shows up here and not before.
	d.setMaxRows(5);
	append(d, 7);
	append(d, 8);
	QCOMPARE(d.rowCount(), 5);
	QCOMPARE(at(d, 0, 0), 400.0);
	QCOMPARE(at(d, 4, 0), 800.0);
}

void TST_WaterfallData::resetKeepsGeometry()
{
	WaterfallData d;
	setup(d, 4);
	append(d, 1);
	append(d, 2);

	d.reset();
	QCOMPARE(d.rowCount(), 0);
	// The row count and bin count survive, so a stop/start reuses the allocation
	// instead of re-mallocing it.
	QCOMPARE(d.maxRows(), 4);
	QCOMPARE(d.value(0.0, 0.0), -DBL_MAX);

	append(d, 9);
	QCOMPARE(d.rowCount(), 1);
	QCOMPARE(at(d, 0, 0), 900.0);
}

void TST_WaterfallData::snapshotMatchesEquivalentAppends()
{
	// setSnapshot takes newest-first; appendRow is fed oldest-first. Same history,
	// so the two must be indistinguishable through value() — which is the property
	// that lets the sim path keep calling setSnapshot while the acq path appends.
	std::vector<QVector<float>> rows;
	for(int i = 3; i >= 1; --i) {
		const std::vector<float> r = mkRow(i);
		rows.push_back(QVector<float>(r.begin(), r.end()));
	}

	WaterfallData snap;
	setup(snap, 4);
	snap.setSnapshot(rows);

	WaterfallData inc;
	setup(inc, 4);
	for(int i = 1; i <= 3; ++i)
		append(inc, i);

	QCOMPARE(snap.rowCount(), inc.rowCount());
	for(int r = 0; r < snap.rowCount(); ++r)
		for(int b = 0; b < kBins; ++b)
			QCOMPARE(at(snap, r, b), at(inc, r, b));

	// And a snapshot leaves the ring appendable, since the acq path may prime from
	// one and then continue incrementally after a fall-behind.
	snap.appendRow(mkRow(4).data(), kBins);
	QCOMPARE(snap.rowCount(), 4);
	QCOMPARE(at(snap, 3, 0), 400.0);
}

void TST_WaterfallData::snapshotDropsRowsBeyondMaxRows()
{
	std::vector<QVector<float>> rows;
	for(int i = 5; i >= 1; --i) { // newest first: 5,4,3,2,1
		const std::vector<float> r = mkRow(i);
		rows.push_back(QVector<float>(r.begin(), r.end()));
	}

	WaterfallData d;
	setup(d, 2);
	d.setSnapshot(rows);
	// Only the newest maxRows are drawable, and the rest are never copied at all.
	QCOMPARE(d.rowCount(), 2);
	QCOMPARE(at(d, 0, 0), 400.0);
	QCOMPARE(at(d, 1, 0), 500.0);
}

void TST_WaterfallData::snapshotAcceptsRaggedRows()
{
	// A short row in the middle of a snapshot used to be read out to the width of
	// rows[0] — a heap overrun on every repaint. It is padded now, and crucially it
	// does not take the width-change branch and wipe the rows before it.
	std::vector<QVector<float>> rows;
	{
		const std::vector<float> newest = mkRow(3);
		rows.push_back(QVector<float>(newest.begin(), newest.end()));
		const std::vector<float> mid = mkRow(2, 3);
		rows.push_back(QVector<float>(mid.begin(), mid.end()));
		const std::vector<float> oldest = mkRow(1);
		rows.push_back(QVector<float>(oldest.begin(), oldest.end()));
	}

	WaterfallData d;
	setup(d, 4);
	d.setSnapshot(rows);

	QCOMPARE(d.rowCount(), 3);
	QCOMPARE(at(d, 0, 0), 100.0);
	QCOMPARE(at(d, 1, 0), 200.0);
	QCOMPARE(at(d, 2, 0), 300.0);
	// The short row's real bins, then its padding — read, not left uninitialised.
	QCOMPARE(at(d, 1, 2), 202.0);
	QCOMPARE(at(d, 1, kBins - 1), static_cast<double>(-FLT_MAX));
}

void TST_WaterfallData::valueRejectsOutOfRange()
{
	WaterfallData d;
	setup(d, 4);
	append(d, 1);
	append(d, 2);

	// Above the newest live row and below the oldest: both outside the filled part
	// of the axis, which Qwt asks about whenever the history is shorter than the
	// axis.
	QCOMPARE(d.value(0.0, -1.0), -DBL_MAX);
	QCOMPARE(d.value(0.0, static_cast<double>(d.rowCount())), -DBL_MAX);
	// Outside the frequency interval.
	QCOMPARE(d.value(-0.1, 0.0), -DBL_MAX);
	QCOMPARE(d.value(1.5, 0.0), -DBL_MAX);
	// Plot width or height of zero really does reach here.
	QCOMPARE(d.value(qQNaN(), 0.0), -DBL_MAX);
	QCOMPARE(d.value(0.0, qInf()), -DBL_MAX);
}

QTEST_GUILESS_MAIN(TST_WaterfallData)
#include "tst_waterfalldata.moc"
