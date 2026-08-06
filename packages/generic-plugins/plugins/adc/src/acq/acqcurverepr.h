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

#ifndef ACQCURVEREPR_H
#define ACQCURVEREPR_H

#include "acqchannelrepr.h"

#include <core/acq_engine/SampleBuffer.h>

#include <QList>
#include <QPointer>
#include <QVector>

#include <functional>
#include <memory>

namespace scopy {
class MenuCombo;
class PlotChannel;

namespace gui {
class MenuPlotAxisRangeControl;
class PlotAutoscaler;
} // namespace gui

namespace adc {

class TimeMeasureManager;

// An ordinary Qwt curve on the row's shared X/Y axes.
//
// Y comes from the channel's key. X is either the manager's shared 0..plotSize-1
// ramp (borrowed, never copied — N channels must not each hold an identical
// ramp) or a second key, which is how an FFT channel gets a frequency axis
// without the channel knowing anything about FFTs.
//
// LIFETIME — the toFloatView aliasing contract. FloatView points into the chunk
// itself when the stream is already Float32, and into the scratch vector
// otherwise; it is valid only while both outlive it
// (core/include/core/acq_engine/SampleBuffer.h:94-100). So m_live/m_liveX and
// m_scratch/m_scratchX are all members here, mirroring
// src/sim/siminstrumentcontroller.h. Three rules follow, and each is a real
// footgun:
//
//   1. Assign the member, then take the view, then consume it, then return. A
//      view must not survive pull().
//   2. Never copy m_live into a local before viewing it. QVector is refcounted,
//      a second handle makes the buffer shared, and any later non-const touch
//      detaches and reallocates under the view. Read m_live const-only.
//   3. If a view's contents must survive the call, memcpy into an owned
//      QVector<float> — never store the pointer.
//
// setSamples() is called with copy=true so Qwt keeps its own buffer and no view
// outlives pull(). If that ever changes to setRawSamples, m_live becomes live
// storage for Qwt and reset() stops being merely cosmetic.
class CurveRepr : public AcqChannelRepr
{
public:
	CurveRepr();
	~CurveRepr() override;

	QString kindName() const override { return QStringLiteral("CURVE"); }

	void attach(AcqPlotRow *row, const QString &name, const QColor &color) override;
	void detach() override;
	void pull(scopy::acq::DataStore *store, const scopy::acq::DataKey &key, int plotSize) override;
	void reset() override;
	std::size_t claimDepth(int plotSize, std::size_t bufferSize) const override;
	QList<scopy::acq::DataKey> extraKeys() const override;
	QWidget *createSettingsWidget(QWidget *parent) override;
	MeasureManagerInterface *measureManager() const override;
	void setEnabled(bool en) override;
	void setColor(const QColor &c) override;
	void setName(const QString &n) override;

	// Timeline the measurements are expressed on. Period, frequency and the rise/fall
	// times are all sample counts divided by this, so a wrong value scales every
	// horizontal measurement — it does not merely mislabel an axis. Unset means 1.0,
	// i.e. measurements come out in samples.
	void setSampleRate(double sr);

	// ---- CurveRepr-specific ------------------------------------------------

	// The shared 0..plotSize-1 ramp, owned by the manager. Borrowed, not copied.
	// The manager only ever resizes it from setPlotSize(), never from inside a pull.
	void setIndexSource(const QVector<float> *idx) { m_indexSrc = idx; }

	// The X stream. Either a real DataKey (pluto_iq_freq for an FFT magnitude channel)
	// or kSampleIndexKey, the sentinel meaning "the manager's 0..plotSize-1 ramp".
	//
	// Empty is treated as the sentinel too, so a channel constructed without an X key
	// draws against sample index — which is what every channel wants by default.
	void setXKey(const scopy::acq::DataKey &k);
	const scopy::acq::DataKey &xKey() const { return m_xKey; }

	// The sentinel X key: a synthetic stream, not something any block writes, so it
	// never appears in DataStore::keys() and can never collide with a real key (no
	// source publishes a key beginning with '$').
	//
	// It exists because the alternative — an empty DataKey meaning "index" — is
	// unrepresentable in a combo box next to the real keys, and the X source is now a
	// user-editable choice rather than something only the controller sets.
	static const scopy::acq::DataKey &sampleIndexKey();

	// Whether `k` names the index ramp rather than a stored stream. True for the
	// sentinel and for an empty key.
	static bool isSampleIndexKey(const scopy::acq::DataKey &k);

	// The span of the X samples last drawn, when X comes from a real stream. False while
	// this curve draws against the index ramp, and false before the first pull that
	// produced data — a curve on a frequency key that has not been written yet must not
	// drag the row's axis to a made-up range.
	//
	// The manager reads this rather than the axis: with X shared per row, the range on
	// screen has to be the union across the row's channels, which no single repr can
	// compute. Recorded during pull() because the window is already in hand there and
	// re-reading the store to answer this would double the copy.
	bool xDataRange(double &min, double &max) const;

	// For PlotAutoscaler and the curve-style control. Null before attach().
	PlotChannel *plotChannel() const { return m_ch; }

	// Every key currently in the store, for the X-source combo the settings page
	// builds. Set by the manager, which is the only thing here holding the store; the
	// repr has no store pointer of its own (pull() is handed one per call).
	void setKeySource(std::function<QList<scopy::acq::DataKey>()> fn) { m_keySource = std::move(fn); }

	// Repopulate the X-source combo from setKeySource(). The manager calls this on
	// keysChanged: keys appear as the pipeline runs, and the settings page is built once
	// and never rebuilt, so a list filled only at construction would stay the empty
	// pre-run set for the life of the channel. No-op before the page exists.
	void refreshKeySources();

private:
	// Follows the enable state: an autoscaler must not keep scaling the row's Y axis
	// to a curve the reader switched off, and PlotAutoscaler::autoscale() walks its
	// channel list without checking isEnabled(). Same pairing as
	// SimInstrumentController::setCurveDriven (src/sim/siminstrumentcontroller.cpp:942).
	void syncAutoscalerChannel();

	QPointer<PlotWidget> m_plot;
	PlotChannel *m_ch{nullptr};
	QPointer<AcqPlotRow> m_row;

	const QVector<float> *m_indexSrc{nullptr};
	// Defaults to the sentinel in the constructor, not left empty, so xKey() answers
	// the combo honestly before anyone calls setXKey().
	scopy::acq::DataKey m_xKey;

	std::function<QList<scopy::acq::DataKey>()> m_keySource;
	// The X-source combo, rebuilt from m_keySource each time it is shown: keys appear as
	// the pipeline runs, so a list built once at page-construction time would be the
	// empty pre-run set forever.
	QPointer<scopy::MenuCombo> m_xCombo;

	// Read windows and their conversion scratch. Members, not locals — see the
	// lifetime note above.
	scopy::acq::SampleVariant m_live, m_liveX;
	QVector<float> m_scratch, m_scratchX;

	// Span of the X samples handed to setSamples() on the last pull, for xDataRange().
	// Invalidated by setXKey() and reset(), so a stale frequency span cannot outlive the
	// key it came from.
	double m_xMin{0.0}, m_xMax{0.0};
	bool m_xRangeValid{false};

	bool m_enabled{true};

	// Built by createSettingsWidget, parented into the returned widget. QPointer
	// because the page belongs to InstrumentTemplate's stack, which outlives this repr
	// on tool teardown in some orders.
	QPointer<scopy::gui::PlotAutoscaler> m_autoscaler;
	QPointer<scopy::gui::MenuPlotAxisRangeControl> m_yCtrl;
	bool m_autoscaleEnabled{false};

	// Created in attach(), because the panels bind to its signals once and the pen
	// colour it stamps on every label is only known from there. Parented to nothing —
	// held by unique_ptr, since a repr is not a QObject and has no children.
	std::unique_ptr<TimeMeasureManager> m_measureMgr;
	double m_sampleRate{1.0};
};

} // namespace adc
} // namespace scopy

#endif // ACQCURVEREPR_H
