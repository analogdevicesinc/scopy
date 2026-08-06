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

#ifndef ACQCHANNELREPR_H
#define ACQCHANNELREPR_H

#include <core/acq_engine/DataKey.h>

#include <QColor>
#include <QList>
#include <QString>
#include <QWidget>

#include <cstddef>
#include <functional>

namespace scopy {
class PlotWidget;

namespace acq {
class DataStore;
}

namespace adc {

class AcqPlotRow;
class MeasureManagerInterface;

// How one channel is drawn. A channel is a DataKey plus one of these plus the GUI
// to edit it; everything else — FFT, scale/offset, thresholds — is an engine block
// writing a derived key the channel points at.
//
// A repr owns its visual (a PlotChannel, a DigitalCurveItem, a whole
// WaterfallPlotWidget) and declares two things the channel cannot know:
//
//   - its read shape: which DataStore call to make in pull(). window() for item
//     reprs, snapshot() for the waterfall.
//   - its depth claim: how many chunks of history it needs.
//
// The repr is fixed for the channel's lifetime, injected into AcqChannel's
// constructor. Changing representation means removing the channel and adding a new
// one — which is what lets the settings page be built once and never rebuilt.
class AcqChannelRepr
{
public:
	virtual ~AcqChannelRepr() = default;

	// Section title on the settings page: "CURVE", "DIGITAL", "WATERFALL".
	virtual QString kindName() const = 0;

	// ---- plot placement ----------------------------------------------------

	// Non-null when this repr *is* a plot rather than an item on one. The manager
	// puts the returned widget in its own exclusive row and hands that row back to
	// attach(), so attach() has one shape for every repr and the manager never
	// needs to know which concrete PlotWidget subclass a repr wants.
	virtual PlotWidget *createOwnPlot(QWidget *parent)
	{
		Q_UNUSED(parent)
		return nullptr;
	}

	// Bind to a row. The row's plot is alive and already in the manager's layout.
	virtual void attach(AcqPlotRow *row, const QString &name, const QColor &color) = 0;

	// Unbind. Must be idempotent, and must tolerate the plot having been destroyed
	// first — plots and channels are siblings under the manager, so their
	// destruction order is unspecified. Hold the plot in a QPointer and open with
	//     if(m_plot.isNull()) { drop pointers; return; }
	// which is the discipline DigitalTrackManager's destructor documents
	// (src/sim/DigitalTrackManager.cpp:38-49).
	virtual void detach() = 0;

	// ---- per cycle ---------------------------------------------------------

	// One cycle's read. The repr chooses its own DataStore call, so the read shape
	// lives where it is implemented rather than being decided by the manager.
	//
	// Anything derived from toFloatView() aliases either the chunk or the scratch
	// vector, so both must be members of the repr and neither the view nor a
	// pointer into it may outlive this call. See acqcurverepr.h.
	virtual void pull(scopy::acq::DataStore *store, const scopy::acq::DataKey &key, int plotSize) = 0;

	// Drop every cached buffer and blank the visual. Called from onStarted(),
	// after AcqInstrument::run() has already done store->clear(): the chunks are
	// gone but the visual still holds the previous run's samples until the first
	// new cycle lands. Mirrors SimInstrumentController::resetLiveBuffers()
	// (src/sim/siminstrumentcontroller.cpp:777-806).
	virtual void reset() = 0;

	// ---- depth -------------------------------------------------------------

	// Chunks of history this representation needs. The channel turns this into
	// DataStore::claimDepth() under its own claimant name.
	virtual std::size_t claimDepth(int plotSize, std::size_t bufferSize) const = 0;

	// Keys this repr reads *besides* the channel's own — a curve's X stream is the
	// only one today. The channel claims the same depth on each, which is not
	// cosmetic: pull() truncates to qMin(x.size, y.size), so an X key left at the
	// default depth of 1 would clip a multi-chunk window down to a single buffer and
	// the curve would silently draw the last 1/Nth of its window.
	//
	// Returned rather than claimed here because a repr knows neither plotSize nor
	// bufferSize, and the claimant name belongs to the channel.
	virtual QList<scopy::acq::DataKey> extraKeys() const { return {}; }

	// A repr whose claimDepth() answer can change on its own — a waterfall's row
	// count is a setting, not a function of plotSize — calls this to say so. The
	// channel installs a callback that emits AcqChannel::depthNeedsReclaim, which
	// the manager turns back into a claim with the plotSize and bufferSize only it
	// knows.
	//
	// A std::function rather than a signal because AcqChannelRepr is deliberately
	// not a QObject: reprs are held by value in a unique_ptr, and the one thing
	// they need to push upwards is not worth a moc pass and a second QObject in
	// every subclass.
	void setReclaimNotifier(std::function<void()> fn) { m_reclaimNotifier = std::move(fn); }

protected:
	void requestReclaim() const
	{
		if(m_reclaimNotifier) {
			m_reclaimNotifier();
		}
	}

public:

	// ---- settings ----------------------------------------------------------

	// The repr's own knobs — axis range, curve style, autoscale, intensity range.
	// These live here rather than on the channel because every one of the existing
	// controls binds to a representation object (MenuPlotAxisRangeControl takes a
	// PlotAxis*, PlotAutoscaler takes a PlotChannel*) and a waterfall has neither.
	//
	// nullptr for a repr with no knobs. Built once by
	// AcqChannel::createSettingsPage() and never rebuilt.
	virtual QWidget *createSettingsWidget(QWidget *parent)
	{
		Q_UNUSED(parent)
		return nullptr;
	}

	// ---- measurements ------------------------------------------------------

	// The repr's measurement manager, or null for a repr with nothing to measure.
	//
	// It lives here rather than on the channel for the same reason the settings do:
	// what a measurement *means* is a property of the representation. Period and rise
	// time are questions about a time-domain curve; a spectrogram has no answer to
	// them, and a digital track's answers would be a different set entirely.
	//
	// Non-null only after attach(). The manager's panels bind to its signals once,
	// which is why it must be created in attach() and not per call.
	virtual MeasureManagerInterface *measureManager() const { return nullptr; }

	virtual void setEnabled(bool en) = 0;

	virtual void setColor(const QColor &c) { Q_UNUSED(c) }
	virtual void setName(const QString &n) { Q_UNUSED(n) }

private:
	std::function<void()> m_reclaimNotifier;
};

} // namespace adc
} // namespace scopy

#endif // ACQCHANNELREPR_H
