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

#ifndef ACQANNOTATIONCHANNEL_H
#define ACQANNOTATIONCHANNEL_H

#include "acqchannel.h"

#include <QPointer>

namespace scopy {
class AnnotationCurve;
class PlotAxis;
class PlotAxisHandle;
class PlotWidget;

namespace adc {

// Decoder annotations as stacked, labelled blocks on the plot's shared digital
// axis — a UART decoder's start/data/parity/stop rows, an SPI decoder's MOSI and
// MISO bytes.
//
// Structurally this is AcqDigitalChannel with a different item: a fixed-height band
// on digitalAxis(), positioned off a draggable PlotAxisHandle, laid out
// proportionally across the shared X axis via setSampleCount(). AnnotationCurve
// is rtti Rtti_PlotUserItem + 42 against DigitalCurveItem's + 43, so the two
// coexist on one axis and a logic track can sit directly above the decode of it.
//
// Three things are genuinely different, and each is why this could not just be a
// flag on AcqDigitalChannel:
//
//   - IT READS THE VARIANT, NOT A CONVERSION. toFloat, toFloatView and toBits all
//     return empty for SampleType::Annotation — an annotation carries no numeric
//     samples. So readData() narrows with latestAs<QVector<Annotation>>() instead,
//     and converts acq::Annotation to gui::AnnotationSpan by hand. (The two structs
//     are deliberately separate: AnnotationCurve lives in gui/, which scopy-core
//     links against, so it cannot name a core type.)
//
//   - ITS DEPTH CLAIM IS 1, NOT ceil(plotSize / bufferSize). SampleBuffer::window()
//     returns an annotation stream as-is from the *newest chunk only*
//     (core/include/core/acq_engine/SampleBuffer.h:147-150), because annotations
//     carry absolute sample offsets and stitching two chunks would need those
//     offsets re-anchored — which is what shiftAnnotations() exists for and what
//     nothing here does. Claiming more chunks would retain history no read can
//     reach.
//
//   - IT HAS NO SINGLE COLOUR. AnnotationCurve buckets by Annotation::klass and
//     picks a stable HSL hash per class, so a UART decode is four colours whether
//     or not anyone asked. onColorChanged() therefore only tints the handle; the
//     channel colour is the band's identity in the rail, not on the plot.
class AcqAnnotationChannel : public AcqChannel
{
	Q_OBJECT
public:
	explicit AcqAnnotationChannel(const Args &args);
	~AcqAnnotationChannel() override;

	QString kindName() const override { return QStringLiteral("ANNOTATIONS"); }

	void reset() override;

protected:
	void attachTo(AcqPlot *plot) override;
	void detachFrom() override;
	void readData(scopy::acq::DataStore *store, int plotSize) override;
	DepthNeed depthNeeded(int plotSize) const override;

	void onEnabledChanged(bool en) override;
	void onColorChanged(const QColor &c) override;

	// Same reason as AcqDigitalChannel: the band hangs off a handle on the plot's shared
	// digital axis, so a private Y scale would have nothing drawn against it.
	bool wantsPooledYAxis() const override { return false; }
	scopy::PlotAxis *ownYAxis(AcqPlot *plot) override;

private:
	QPointer<PlotWidget> m_plot;

	// Not a QPointer: AnnotationCurve is a QwtPlotItem, not a QObject.
	AnnotationCurve *m_item{nullptr};
	// Is a QPointer: AxisHandle parents itself to the canvas, so a canvas teardown
	// takes it with no notice to us.
	QPointer<PlotAxisHandle> m_handle;
};

} // namespace adc
} // namespace scopy

#endif // ACQANNOTATIONCHANNEL_H
