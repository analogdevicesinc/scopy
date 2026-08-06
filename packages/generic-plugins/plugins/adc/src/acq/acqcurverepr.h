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

#include <QPointer>
#include <QVector>

#include <memory>

namespace scopy {
class PlotChannel;

namespace gui {
class MenuPlotAxisRangeControl;
class PlotAutoscaler;
} // namespace gui

namespace adc {

class TimeMeasureManager;

// An ordinary Qwt curve on the row's shared X/Y axes.
//
// Y comes from the channel's key. X is the manager's shared 0..plotSize-1 ramp,
// borrowed and never copied — N channels must not each hold an identical ramp.
//
// LIFETIME — the toFloatView aliasing contract. FloatView points into the chunk
// itself when the stream is already Float32, and into the scratch vector
// otherwise; it is valid only while both outlive it
// (core/include/core/acq_engine/SampleBuffer.h:94-100). So m_live and m_scratch
// are both members here, mirroring src/sim/siminstrumentcontroller.h. Three
// rules follow, and each is a real footgun:
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

	// For PlotAutoscaler and the curve-style control. Null before attach().
	PlotChannel *plotChannel() const { return m_ch; }

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

	// The read window and its conversion scratch. Members, not locals — see the
	// lifetime note above.
	scopy::acq::SampleVariant m_live;
	QVector<float> m_scratch;

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
