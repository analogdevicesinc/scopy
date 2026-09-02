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

#ifndef ACQCURVECHANNEL_H
#define ACQCURVECHANNEL_H

#include "acqchannel.h"

#include <QPointer>
#include <QVector>

namespace scopy {
class PlotChannel;
class PlotWidget;

namespace adc {

// An ordinary Qwt curve on this channel's own X/Y axes, out of the plot's pool.
//
// Y comes from the channel's key. X comes from xKey() — the stream the reader picked,
// or the one the descriptor named (FFT bin frequencies) — or, when that is empty, the
// plot's 0..plotSize-1 ramp, borrowed and never copied: N channels must not each hold
// an identical ramp.
//
// LIFETIME — the toFloatView aliasing contract. FloatView points into the chunk
// itself when the stream is already Float32, and into the scratch vector
// otherwise; it is valid only while both outlive it
// (core/include/core/acq_engine/SampleBuffer.h:94-100). So m_live and m_scratch
// are both members here, mirroring src/sim/siminstrumentcontroller.h. Three
// rules follow, and each is a real footgun:
//
//   1. Assign the member, then take the view, then consume it, then return. A
//      view must not survive readData().
//   2. Never copy m_live into a local before viewing it. QVector is refcounted,
//      a second handle makes the buffer shared, and any later non-const touch
//      detaches and reallocates under the view. Read m_live const-only.
//   3. If a view's contents must survive the call, memcpy into an owned
//      QVector<float> — never store the pointer.
//
// setSamples() is called with copy=true so Qwt keeps its own buffer and no view
// outlives readData(). If that ever changes to setRawSamples, m_live becomes live
// storage for Qwt and reset() stops being merely cosmetic.
class AcqCurveChannel : public AcqChannel
{
	Q_OBJECT
public:
	explicit AcqCurveChannel(const Args &args);
	~AcqCurveChannel() override;

	QString kindName() const override { return QStringLiteral("CURVE"); }

	void reset() override;

	// For the axes' autoscalers and the curve-style control. Null before attach().
	PlotChannel *plotChannel() const override { return m_ch; }

protected:
	void attachTo(AcqPlot *plot) override;
	void detachFrom() override;
	void readData(scopy::acq::DataStore *store, int plotSize) override;
	DepthNeed depthNeeded(int plotSize) const override;
	QWidget *createKindSettings(QWidget *parent) override;

	void onEnabledChanged(bool en) override;
	void onColorChanged(const QColor &c) override;
	void onNameChanged(const QString &n) override;
	void onSampleRateChanged(double sr) override;

private:
	QPointer<PlotWidget> m_plot;
	PlotChannel *m_ch{nullptr};

	// The blank-curve dummy sample. A member so its address stays valid for as long as
	// the curve might read it — some Qwt paths dereference the data pointer even for
	// empty ranges (src/sim/siminstrumentcontroller.cpp:798-803).
	const float m_kZero;

	// The read window and its conversion scratch. Members, not locals — see the
	// lifetime note above.
	scopy::acq::SampleVariant m_live;
	QVector<float> m_scratch;
	// The X window, read from the stream xKey() names — which is now every case,
	// including sample index and time: both read the engine's ramp. Owned rather than
	// viewed, because a second FloatView over the same scratch would alias the Y data.
	QVector<float> m_xData;

	// The derived X for the two index-based modes, and the inputs it was built from.
	//
	// Both are affine functions of the ramp window, so they are rebuilt only when their
	// inputs change — a plot width change, a rate change, or history filling in — and
	// not per frame. That is the whole point of the ramp being one chunk written once:
	// no arithmetic on the display path.
	//
	//   SampleIndex — the window rebased to its own first value, since a plot narrower than
	//                 the ramp reads its tail (3072..4095) and the window must read
	//                 0..plotSize-1. The base is the start of the *window*, not of the
	//                 samples that have arrived, so a partial window keeps its slot numbers
	//                 (plotSize-n..plotSize-1) and its curve sits in the right part of the
	//                 plot. These are only the values; the axis range is pinned to the
	//                 window independently, at the end of readData(). Left empty when the
	//                 window already starts at 0, the common case: the store's own vector
	//                 is then used with no copy at all.
	//   Time        — that same rebased index over sampleRate(). Values are small, so
	//                 Float32 is exact here in a way an absolute sample counter would not be.
	QVector<float> m_derivedX;
	// What m_derivedX was built for. The mode is in the tuple because switching between
	// sample index and time reads the same window and would otherwise reuse the wrong
	// cache; the size catches a width change and the window's first value catches the ramp
	// window scrolling.
	int m_derivedMode{-1};
	float m_derivedFirst{0.0f};
	int m_derivedSize{0};
	double m_derivedRate{0.0};
};

} // namespace adc
} // namespace scopy

#endif // ACQCURVECHANNEL_H
