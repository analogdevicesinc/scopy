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

#ifndef ACQANNOTATIONREPR_H
#define ACQANNOTATIONREPR_H

#include "acqchannelrepr.h"

#include <QPointer>

namespace scopy {
class AnnotationCurve;
class PlotAxisHandle;

namespace adc {

// Decoder annotations as stacked, labelled blocks on the row's shared digital
// axis — a UART decoder's start/data/parity/stop rows, an SPI decoder's MOSI and
// MISO bytes.
//
// Structurally this is DigitalRepr with a different item: a fixed-height band on
// digitalAxis(), positioned off a draggable PlotAxisHandle, laid out
// proportionally across the shared X axis via setSampleCount(). AnnotationCurve
// is rtti Rtti_PlotUserItem + 42 against DigitalCurveItem's + 43, so the two
// coexist on one axis and a logic track can sit directly above the decode of it.
//
// Three things are genuinely different, and each is why this could not just be a
// flag on DigitalRepr:
//
//   - IT READS THE VARIANT, NOT A CONVERSION. toFloat, toFloatView and toBits all
//     return empty for SampleType::Annotation — an annotation carries no numeric
//     samples. So pull() narrows with latestAs<QVector<Annotation>>() instead, and
//     converts acq::Annotation to gui::AnnotationSpan by hand. (The two structs
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
//     or not anyone asked. setColor() therefore only tints the handle; the channel
//     colour is the band's identity in the rail, not on the plot.
class AnnotationRepr : public AcqChannelRepr
{
public:
	AnnotationRepr();
	~AnnotationRepr() override;

	QString kindName() const override { return QStringLiteral("ANNOTATIONS"); }

	void attach(AcqPlotRow *row, const QString &name, const QColor &color) override;
	void detach() override;
	void pull(scopy::acq::DataStore *store, const scopy::acq::DataKey &key, int plotSize) override;
	void reset() override;
	std::size_t claimDepth(int plotSize, std::size_t bufferSize) const override;
	void setEnabled(bool en) override;
	void setColor(const QColor &c) override;

private:
	QPointer<PlotWidget> m_plot;
	QPointer<AcqPlotRow> m_row;

	// Not a QPointer: AnnotationCurve is a QwtPlotItem, not a QObject.
	AnnotationCurve *m_item{nullptr};
	// Is a QPointer: AxisHandle parents itself to the canvas, so a canvas teardown
	// takes it with no notice to us.
	QPointer<PlotAxisHandle> m_handle;

	QColor m_color;
	bool m_enabled{true};
};

} // namespace adc
} // namespace scopy

#endif // ACQANNOTATIONREPR_H
