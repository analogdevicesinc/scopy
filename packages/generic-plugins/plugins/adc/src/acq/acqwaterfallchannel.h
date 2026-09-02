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

#ifndef ACQWATERFALLCHANNEL_H
#define ACQWATERFALLCHANNEL_H

#include "acqchannel.h"

#include <QPointer>
#include <QVector>

namespace scopy {
class WaterfallPlotWidget;

namespace adc {

// A spectrogram of the key's chunk history: one pixel row per retained chunk.
//
// This is the kind the base's interface was shaped for, because it is the one that
// is not an item on a shared plot:
//
//   - it *is* a plot. The Waterfall plot kind's registry factory builds the
//     WaterfallPlotWidget, so a waterfall channel only ever lands on a plot that is
//     already one — and the manager never names this class or includes its header;
//     both axes therefore come from the widget, not from the plot's pool;
//   - it reads snapshot(), not window(). History is the vertical axis here, so the
//     depth claim is a row count rather than ceil(plotSize / bufferSize) — plotSize
//     is ignored entirely;
//   - the row count is a *setting*, so unlike every other kind its claim changes
//     without the manager doing anything. requestReclaim() is what closes that
//     loop.
//
// It reads incrementally, which is what makes the row count affordable. Only the
// chunks pushed since the last cycle are new, so those are the only ones appended;
// the rest are already in the widget's ring. Two things make that exact without a
// per-key sequence counter in the DataStore:
//
//   - a retained chunk's QVector allocation is stable while it is in the buffer, so
//     its constData() pointer identifies it. m_lastChunk is the newest one already
//     drawn, and scanning snapshot() for it gives the exact count of new chunks
//     while touching no sample data;
//   - the scan is needed rather than a plain "one row per cycle" because Continuous
//     mode free-runs and only rate-limits cycleComplete
//     (AcquisitionEngine.cpp:432-441), so several chunks per cycle is the norm.
//
// No match means we fell behind by more than the retained depth (or this is the
// first cycle, or a reset): the whole history is redrawn, which is correct rather
// than merely tolerable — nothing is duplicated and the only rows lost are ones the
// store itself already evicted.
//
// The claim is exactly m_rows because that is what the ring can show; retaining
// chunks nobody paints would pay for a deeper scan for nothing.
class AcqWaterfallChannel : public AcqChannel
{
	Q_OBJECT
public:
	explicit AcqWaterfallChannel(const Args &args);
	~AcqWaterfallChannel() override;

	QString kindName() const override { return QStringLiteral("WATERFALL"); }

	void reset() override;

	void setRows(int rows);
	int rows() const { return m_rows; }

protected:
	void attachTo(AcqPlot *plot) override;
	void detachFrom() override;
	void readData(scopy::acq::DataStore *store, int plotSize) override;
	DepthNeed depthNeeded(int plotSize) const override;
	QWidget *createKindSettings(QWidget *parent) override;

	void onEnabledChanged(bool en) override;

	// Neither side comes from the pool: WaterfallPlotWidget owns an inverted time axis
	// and an Hz axis, and a spectrogram drawn against anything else is not a
	// spectrogram. Both AcqAxis still exist, wrapping the widget's own pair, so the two
	// sections are on the page with the constraint stated rather than absent for this
	// one kind.
	bool wantsPooledXAxis() const override { return false; }
	bool wantsPooledYAxis() const override { return false; }

private:
	// The plot's own widget, found by attachTo(), never built here — owned by the dock the
	// manager wrapped it in, which is why this is a QPointer and not a unique_ptr. Null
	// when this kind was put on a plot that is not a waterfall.
	QPointer<WaterfallPlotWidget> m_wf;

	// Enough history to see a slow drift. The upper bound is no longer a frame-budget
	// limit — the read is incremental and the widget's ring is preallocated, so the
	// per-cycle cost does not scale with it — only a bound on memory and on how much
	// history is meaningful to look at.
	const int m_kMinRows;
	const int m_kMaxRows;

	int m_rows{200};
	double m_minDb{-120.0};
	double m_maxDb{0.0};

	// The last frequency range pushed, so it is only pushed again when it actually
	// changes (an FFT reconfigure). setFrequencyRange() rescales the axis and
	// invalidates the raster cache, which is not worth paying every cycle for a
	// number that is constant across a run.
	double m_freqStart{0.0};
	double m_freqStop{0.0};

	// Identity of the newest chunk already appended — see the class comment. Void
	// because it is only ever compared, never dereferenced: the chunk it came from
	// may well have been evicted by the time it is looked at again, which is exactly
	// the case the no-match branch handles.
	const void *m_lastChunk{nullptr};

	// toFloatView's conversion scratch, a member so it is allocated once rather than
	// per row. The view aliases it, so only one view may be live at a time — safe
	// here because appendRowDeferred() copies before returning, so each row is
	// consumed before the next overwrites the scratch.
	QVector<float> m_scratch;

	void updateFrequencyRange(scopy::acq::DataStore *store);
};

} // namespace adc
} // namespace scopy

#endif // ACQWATERFALLCHANNEL_H
