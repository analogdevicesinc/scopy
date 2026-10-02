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

#ifndef ACQCHANNEL_H
#define ACQCHANNEL_H

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datakey.h>
// Not forward-declarable: DepthNeed below aliases DataStore::Claim.
#include <core/acq_engine/datastore.h>
#include <core/acq_engine/samplebuffer.h>

#include <QColor>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>

#include <cstddef>
#include <optional>
#include <variant>

namespace scopy {
class InstrumentTemplate;
class PlotAxis;
class PlotChannel;
class PlotWidget;

namespace adc {

class AcqAxis;
class AcqPlot;

// What a channel needs retained. The store's own claim type — a channel's requirement
// and a claim are the same thing said twice. See DataStore::Claim for the two units.
using DepthNeed = scopy::acq::DataStore::Claim;

// A DataKey, drawn.
//
// A channel is a key, the producer's descriptor for it, and the identity to edit
// both. There is no FFT here, no scale/offset, no YMode: those are engine blocks,
// and a channel that wants a derived stream points at the key the block writes
// (pluto_iq_fft rather than pluto_voltage0_raw).
//
// This is an abstract base and the subclasses *are* the representations —
// AcqCurveChannel, AcqDigitalChannel, AcqAnnotationChannel, AcqWaterfallChannel.
// There is deliberately no separate repr object: it was fixed at construction, so
// the two had identical lifetimes and each was always exactly one of the other,
// and the split cost a std::function standing in for a signal plus a forwarding
// setter per property.
//
// Key and kind are both fixed for the channel's lifetime. Changing either means
// removing this channel and adding a new one — which is what lets the settings page
// be built once and never rebuilt (gui/include/gui/instrumenttemplate.h:206).
//
// SUBCLASS CONTRACT — the public surface here is non-virtual and the overridables
// are protected, so the base keeps its own invariants (a disabled channel never
// pays for a read, the owner and the axis pair always match the last attach) and a subclass cannot
// forget to maintain them. Implement readData/depthNeeded/attachTo/detachFrom and
// leave the guards alone.
class AcqChannel : public QObject
{
	Q_OBJECT
public:
	// Constructor arguments as one struct, so the registry can hand every kind the
	// same thing and a new field does not touch four constructors and a factory
	// signature.
	struct Args
	{
		scopy::acq::DataStore *store{nullptr};
		// Only passed through to the axes, which read stream descriptors and the
		// declared-key list off it for their source pickers. Sample reads go to
		// `store`.
		scopy::acq::AcquisitionEngine *engine{nullptr};
		scopy::acq::DataKey key;
		// The X source the reader picked when adding the channel. Empty means "use
		// info.xKey if the producer named one, otherwise the plot's sample-index ramp" —
		// so an explicit choice here wins over the producer's recommendation, which is
		// the general rule in this codebase.
		scopy::acq::DataKey xKey;
		// The producer's descriptor. Label, colour slot, unit, sample rate and X key
		// all come from here rather than from setters the manager calls afterwards.
		scopy::acq::StreamInfo info;
		// From the manager, and the identity behind both the depth claimant and the
		// menu page id. Neither may be derived from the key or the name — see
		// claimant() below.
		int uid{0};
		QObject *parent{nullptr};
	};

	explicit AcqChannel(const Args &args);
	~AcqChannel() override;

	const scopy::acq::DataKey &key() const { return m_key; }

	// The stream supplying X, or an empty key meaning the plot's 0..plotSize-1 ramp.
	//
	// Unlike key(), this is *not* fixed for the channel's life: the Y key is the
	// channel's identity (which is what lets the settings page be built once), but
	// which stream the horizontal axis is scaled against is a view decision, and
	// retargeting it is a re-claim rather than a rebuild. It follows the X AcqAxis's
	// source.
	const scopy::acq::DataKey &xKey() const { return m_xKey; }

	const scopy::acq::StreamInfo &info() const { return m_info; }
	QString name() const { return m_name; }
	QColor color() const { return m_color; }
	bool isEnabled() const { return m_enabled; }
	int uid() const { return m_uid; }

	// Section title on the settings page: "CURVE", "DIGITAL", "WATERFALL".
	virtual QString kindName() const = 0;

	// The DataStore claimant, "acqch-<uid>". Stable for the channel's life, and
	// deliberately not derived from the key or the name:
	//
	//   - claims are keyed (key, claimant) and a claim *replaces* that
	//     claimant's previous claim, so two channels on one key sharing a claimant
	//     string would silently clobber each other;
	//   - names are user-editable, and a name-derived claimant orphans its old claim
	//     on rename, pinning memory at the high-water mark with no way to release it.
	const QString &claimant() const { return m_claimant; }

	// The InstrumentTemplate menu page / rail row id, "acqch:<uid>".
	QString menuId() const;

	// ---- plot placement ------------------------------------------------------

	void attach(AcqPlot *plot);
	void detach();

	// The plot this channel is attached to, or null once detached.
	AcqPlot *plotOwner() const { return m_owner; }

	// This channel's two axes, or null before attach(). X and Y are the same class at
	// two QwtAxis positions — see acqaxis.h; there is deliberately no asymmetry here.
	AcqAxis *xAxis() const { return m_xAxis; }
	AcqAxis *yAxis() const { return m_yAxis; }

	// Repopulate both source pickers from the store. The manager calls this when the
	// store's key set changes, so a stream that appeared after the settings page was
	// built is still selectable.
	void refreshAxisSourceChoices();

	// The PlotChannel the axes' autoscalers measure, or null for a kind whose visual
	// is a QwtPlotItem (digital, annotation) or a whole widget (waterfall). Null is
	// what suppresses the AUTOSCALE switch — an autoscaler with nothing to measure is
	// a control that does nothing.
	virtual PlotChannel *plotChannel() const;

	// ---- per cycle -----------------------------------------------------------

	// One cycle, drawing `plotSize` samples across the visible width. A no-op while
	// disabled: a disabled channel must not pay for a window copy, and its visual is
	// already hidden.
	//
	// A plain int, not a geometry struct: the visible width is the only thing a channel
	// does not decide for itself. There is no buffer size to pass — a depth claim is
	// registered in samples and the DataStore converts — and no index ramp, since the
	// sample-index X source is a real store stream the engine writes
	// (AcquisitionEngine::indexRampKey()) and is read like any other X.
	//
	// `snap` is a trigger fire's window set, keyed by DataKey::key: draw *this* window and
	// not whatever is in the store now. The processor has already re-anchored each window
	// so the sample that fired sits at the requested split, so re-reading the store would
	// draw a window the reported fire index does not describe. Borrowed for the call only.
	void pull(int plotSize, const QMap<QString, scopy::acq::SampleVariant> *snap = nullptr);

	// Drop every cached buffer and blank the visual. Called from onStarted(), after
	// AcqInstrument::run() has already done store->clear(): the chunks are gone but
	// the visual still holds the previous run's samples until the first new cycle
	// lands.
	virtual void reset() = 0;

	// (Re)register the depth claims on this channel's keys — the Y key, and the X key
	// too when X is a stream. Idempotent — a claim replaces rather than accumulates,
	// so no release is needed first. Correct even when a key does not exist yet:
	// DataStore::write() applies pending claims on the first push, so claiming early
	// is what makes the *first* window full-depth instead of one chunk.
	//
	// Only plotSize can change what this asks for, so the manager calls it on a
	// plotSize change and nothing else; a claim in samples needs no re-registering
	// when the acquisition buffer is resized.
	void reclaimDepth(int plotSize);

	// Run state, for the axes. Replot and data are the manager's business; these exist
	// so a stop leaves the axes scaled to the last frame rather than to the frame the
	// autoscaler happened to be on when its timer stopped.
	void onStarted();
	void onStopped();

	// ---- presence ------------------------------------------------------------

	// Whether the key currently exists in the store. A channel whose key vanished —
	// the reader disabled that source channel, or a decoder stopped producing — is
	// never deleted: the key usually comes back on the next run, and deleting would
	// take the reader's per-channel settings with it. It is disabled and greyed
	// instead, and restored to the enable state it had when the key returns.
	bool isKeyPresent() const { return m_keyPresent; }
	void setKeyPresent(bool present);

	// ---- settings ------------------------------------------------------------
	//
	// There is deliberately no measurement API here — no manager, no selector, no
	// per-cycle measure() call. Measurements are not part of this instrument yet, and a
	// hook with nothing behind it is a hook that gets maintained for nothing.

	// A generic CHANNEL section (name, colour, key readout) stacked over the kind's
	// own section, the way Block::withBaseSettings() composes
	// (core/src/acq_engine/block.cpp:59-70). Built once; the caller hands it to
	// InstrumentTemplate::addMenuPage.
	QWidget *createSettingsPage(InstrumentTemplate *it, QWidget *parent = nullptr);

	// The timeline this channel's samples are on. Normally the producer's, arriving
	// through StreamInfo::sampleRate; this is the fallback for a stream whose producer
	// declared none (a raw ADC channel — the source does not know the device rate),
	// applied by the manager. Ignored once the descriptor carries one: the producer's own
	// statement wins over the view's default.
	void setFallbackSampleRate(double sr);

	// Samples per second, or 1.0 if nothing knows. The divisor of the "time (s)" X mode,
	// which reads in samples when it is 1.0 — wrong-but-readable rather than a zero
	// divisor.
	double sampleRate() const { return m_sampleRate; }

public Q_SLOTS:
	void setEnabled(bool en);
	void setName(const QString &n);
	void setColor(const QColor &c);

Q_SIGNALS:
	void enabledChanged(bool);
	void nameChanged(QString);
	void colorChanged(QColor);
	void keyPresentChanged(bool);

	// This channel's read requirement changed on its own — a waterfall's row count is
	// a setting, not a function of plotSize, and an axis retargeted to another stream
	// moves which keys need depth — so the manager must re-claim with the geometry only
	// it knows, and release the claim on any key that dropped out. `droppedKey` is the
	// key that is no longer read, or an empty key when nothing was dropped.
	void depthNeedsReclaim(scopy::acq::DataKey droppedKey = scopy::acq::DataKey());

	// The Delete channel button on the settings page. Connected queued, so the button
	// survives its own click: the manager destroys this channel — and with it the page
	// and the button — from the handler.
	void removeRequested();

protected:
	// ---- to implement --------------------------------------------------------

	// Bind to a plot. The plot's widget is alive and already in the manager's layout,
	// and this channel's axis pair has already been acquired — build the visual against
	// xAxis()->plotAxis() and yAxis()->plotAxis(). Name, colour and descriptor are
	// already this object's own members.
	virtual void attachTo(AcqPlot *plot) = 0;

	// Unbind. Must be idempotent, and must tolerate the plot having been destroyed
	// first — plots and channels are siblings under the manager, so their destruction
	// order is unspecified. Hold the plot in a QPointer and open with
	//     if(m_plot.isNull()) { drop pointers; return; }
	virtual void detachFrom() = 0;

	// One cycle's read. The subclass chooses its own DataStore call — window() for
	// items, snapshot() for the waterfall, latestAs<>() for annotations — so the read
	// shape lives where it is implemented rather than being decided by the manager.
	//
	// `store` is non-null and this channel is enabled; the base checked both.
	//
	// Anything derived from toFloatView() aliases either the chunk or the scratch
	// vector, so both must be members and neither the view nor a pointer into it may
	// outlive this call. See acqcurvechannel.h.
	virtual void readData(scopy::acq::DataStore *store, int plotSize) = 0;

	// History this representation needs, in samples or in chunks — see DepthNeed.
	virtual DepthNeed depthNeeded(int plotSize) const = 0;

	// The kind's own knobs — curve style, intensity range. Axis range and autoscale are
	// *not* here any more: they are the same four controls on both sides and they live
	// in AcqAxis, which the base puts on the page for X and Y alike. nullptr for a kind
	// with no knobs.
	virtual QWidget *createKindSettings(QWidget *parent);

	// Whether this kind takes the plot's axis for its source on each side. A kind whose Y is
	// the plot's shared digital axis wants only X; a waterfall, which owns both its axes,
	// wants neither. A false answer does not remove the AcqAxis — it wraps an axis the kind
	// names instead, source-fixed, so the settings section stays visible with the
	// constraint stated.
	virtual bool wantsPooledXAxis() const { return true; }
	virtual bool wantsPooledYAxis() const { return true; }

	// The axis a kind that declined wants wrapped instead. Called only when the
	// corresponding wantsPooled*Axis() is false; null falls back to the plot widget's own
	// built-in axis for that side.
	virtual scopy::PlotAxis *ownXAxis(AcqPlot *plot);
	virtual scopy::PlotAxis *ownYAxis(AcqPlot *plot);

	// The reader retargeted this side's source, so the PlotAxis drawn against changed. Move
	// whatever was built against the old one — a PlotChannel, a handle — onto `axis`. Only
	// for a side that takes the plot's source axis.
	virtual void onPlotAxisChanged(bool horizontal, scopy::PlotAxis *axis);

	// Hooks for the three base-owned properties. Called after the member is updated
	// and before the signal, and only when the value actually changed.
	virtual void onEnabledChanged(bool en);
	virtual void onColorChanged(const QColor &c);
	virtual void onNameChanged(const QString &n);
	virtual void onSampleRateChanged(double sr);

	// Say that depthNeeded() would now answer differently.
	void requestReclaim() { Q_EMIT depthNeedsReclaim(scopy::acq::DataKey()); }

	// ---- reads, snapshot-aware -----------------------------------------------
	//
	// Use these instead of store->window()/latest(): on a trigger fire they return that
	// fire's window for the key, and otherwise they are exactly the store call they
	// replace. Reading the store directly still works — it just keeps drawing the live
	// window on a fire, which is the bug these exist to close.
	//
	// The fire's windows are already the requested length, so `plotSize` is only the
	// fallback path's argument. An empty Float32 vector for an absent key, matching
	// DataStore::window().
	scopy::acq::SampleVariant windowFor(const scopy::acq::DataKey &k, int plotSize) const;
	QVector<float> windowFloatFor(const scopy::acq::DataKey &k, int plotSize) const;

	// nullopt when neither the fire nor the store has `k`, so a caller can tell absent
	// from empty.
	std::optional<scopy::acq::SampleVariant> latestFor(const scopy::acq::DataKey &k) const;

	// latestFor() narrowed to one alternative, DataStore::latestAs<>()'s counterpart:
	// nullopt on a type mismatch, because the narrowing is also the check that the key is
	// a stream the caller can draw.
	template <class T>
	std::optional<T> latestAsFor(const scopy::acq::DataKey &k) const
	{
		std::optional<scopy::acq::SampleVariant> v = latestFor(k);
		if(!v || !std::holds_alternative<T>(*v)) {
			return std::nullopt;
		}
		return std::get<T>(std::move(*v));
	}

	// For a subclass that needs the store outside readData(). Null once the store has
	// been destroyed. Out of line because QPointer::data() instantiates against the
	// complete type, which this header deliberately does not include.
	scopy::acq::DataStore *store() const;

private:
	// Build and destroy this channel's axis pair. Called by attach()/detach() around the
	// subclass's attachTo()/detachFrom(), so a kind can build its visual against the axes
	// and still be unhooked before the wrappers go.
	void acquireAxes();
	void releaseAxes();

	// Put `side` on the plot's axis for its current source, moving the kind's visual with it.
	void moveToSourceAxis(AcqAxis *side, int position, bool wantsPooled);

	QPointer<scopy::acq::DataStore> m_store;
	// Borrowed, for the axes' source pickers. Never driven from here.
	QPointer<scopy::acq::AcquisitionEngine> m_engine;
	const scopy::acq::DataKey m_key;
	// Not const: follows the X axis's source. See xKey().
	scopy::acq::DataKey m_xKey;
	const scopy::acq::StreamInfo m_info;
	QString m_name;
	QColor m_color;

	const int m_uid;
	const QString m_claimant;

	AcqPlot *m_owner{nullptr}; // not owned

	// Ours, parented to this — raw pointers because they are created and destroyed in
	// acquireAxes/releaseAxes as a pair and never outlive us. The PlotAxis each one
	// wraps is the plot's, borrowed and returned; see acqaxis.h.
	AcqAxis *m_xAxis{nullptr};
	AcqAxis *m_yAxis{nullptr};

	// The fire window set for the duration of one pull(), or null. Borrowed.
	const QMap<QString, scopy::acq::SampleVariant> *m_snap{nullptr};

	bool m_enabled{true};
	bool m_keyPresent{true};
	// The enable state to restore when a vanished key comes back, so a reader who had
	// this channel switched off does not find it switched on after a re-run.
	bool m_enabledBeforeVanish{true};

	double m_sampleRate{1.0};
};

} // namespace adc
} // namespace scopy

#endif // ACQCHANNEL_H
