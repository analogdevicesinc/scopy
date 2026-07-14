#ifndef SCOPY_ADC_DECODEROVERLAY_H
#define SCOPY_ADC_DECODEROVERLAY_H

#include <QHash>
#include <QObject>
#include <QPointF>
#include <QPointer>

#include "acq_engine/DataKey.h"

class QEvent;

namespace scopy {

class AnnotationCurve;
class PlotAxisHandle;
class PlotWidget;

namespace acq {
class ExternalDecoderProcessor;
class DataStore;
} // namespace acq

namespace adc {

// Subscribes to ExternalDecoderProcessor::cycleProduced, reads the produced
// annotations from the DataStore and forwards them to an AnnotationCurve for
// on-plot drawing. Each registered decoder gets its own annotation band
// rendered against a caller-supplied PlotAxis (its interval defines the
// band extent). Drag repositioning of the band is a separate concern handled
// outside the overlay by whoever owns the axis.
class DecoderOverlay : public QObject
{
	Q_OBJECT
public:
	DecoderOverlay(PlotWidget *plot, scopy::acq::DataStore *store,
		       QObject *parent = nullptr);
	~DecoderOverlay() override;

	// Register a decoder for drawing. The caller supplies the
	// PlotAxisHandle whose scale-space position defines the top of this
	// decoder's annotation band; the overlay attaches an AnnotationCurve
	// that reads the handle's position on every redraw.
	void registerDecoder(scopy::acq::ExternalDecoderProcessor *proc,
			     const scopy::acq::DataKey &outKey,
			     scopy::PlotAxisHandle *handle,
			     const QString &title);

	// Remove the AnnotationCurve associated with the given decoder output
	// key: detach from the plot, delete the curve and drop it from the
	// internal map. Safe to call for unknown keys.
	void unregisterDecoder(const scopy::acq::DataKey &outKey);

	// Push the total sample count into every registered AnnotationCurve so
	// annotations are laid out proportionally across the current plot
	// x-axis range. Pass 0 to revert to sample-index mode.
	void setSampleCount(quint64 n);

	void clear();

	// QObject
	bool eventFilter(QObject *watched, QEvent *ev) override;

private Q_SLOTS:
	void onCycleProduced(scopy::acq::DataKey outKey);

private:
	// Look up the annotation under the given canvas-pixel position,
	// scanning only currently visible AnnotationCurves. Returns the
	// pretty-formatted tooltip text (empty if nothing hit).
	QString tooltipForCanvasPos(const QPointF &canvasPos) const;

	QPointer<PlotWidget>            m_plot;
	QPointer<scopy::acq::DataStore> m_store;

	// Keyed by the decoder's output DataKey.
	QHash<scopy::acq::DataKey, AnnotationCurve *> m_curves;
};

} // namespace adc
} // namespace scopy

#endif
