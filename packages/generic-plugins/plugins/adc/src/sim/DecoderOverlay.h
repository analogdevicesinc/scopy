#ifndef SCOPY_ADC_DECODEROVERLAY_H
#define SCOPY_ADC_DECODEROVERLAY_H

#include <QHash>
#include <QObject>
#include <QPoint>
#include <QPointF>
#include <QPointer>
#include <QString>
#include <QTimer>

#include "acq_engine/DataKey.h"

class QEvent;

namespace scopy {

class AnnotationCurve;
class PlotAxis;
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

	// Optional dedicated y-axis for annotation bands. If set, newly
	// registered AnnotationCurves are attached to this axis instead of
	// the plot's main y-axis (used by the mixed-signal digital area).
	void setAnnotationYAxis(scopy::PlotAxis *axis) { m_annYAxis = axis; }

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
	QPointer<scopy::PlotAxis>       m_annYAxis;

	// Keyed by the decoder's output DataKey.
	QHash<scopy::acq::DataKey, AnnotationCurve *> m_curves;

	// Hover-timer driven tooltip. Tooltip is only shown after the mouse
	// has been still for m_hoverTimer.interval() ms on the canvas. This
	// prevents a synthetic-MouseMove ↔ QToolTip::showText feedback loop
	// (the tooltip window generating Leave/Enter events which used to
	// trigger a fresh showText → new QTipLabel → OOM). Also suppressed
	// while the wheel is turning so magnify zoom stays smooth.
	void   showPendingTooltip();
	QTimer m_hoverTimer;
	QPoint m_pendingCanvasPos;
	QPoint m_pendingGlobalPos;
	QString m_lastTip;
	QPoint  m_lastTipGlobal;
};

} // namespace adc
} // namespace scopy

#endif
