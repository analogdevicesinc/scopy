#ifndef SCOPY_ADC_DECODEROVERLAY_H
#define SCOPY_ADC_DECODEROVERLAY_H

#include <QHash>
#include <QObject>
#include <QPointer>

#include "acq_engine/DataKey.h"

namespace scopy {

class AnnotationCurve;
class PlotAxis;
class PlotWidget;

namespace acq {
class ExternalDecoderProcessor;
class DataStore;
} // namespace acq

namespace adc {

// Subscribes to ExternalDecoderProcessor::cycleProduced, reads the produced
// annotations from the DataStore and forwards them to an AnnotationCurve for
// on-plot drawing. Each registered decoder gets its own PlotAxis (vertical
// band) and its own AnnotationCurve instance.
class DecoderOverlay : public QObject
{
	Q_OBJECT
public:
	DecoderOverlay(PlotWidget *plot, scopy::acq::DataStore *store,
		       QObject *parent = nullptr);
	~DecoderOverlay() override;

	// Register a decoder for drawing. The caller supplies the PlotAxis
	// that defines the vertical band this decoder will occupy; a new
	// AnnotationCurve is created, attached to the plot and stored here.
	void registerDecoder(scopy::acq::ExternalDecoderProcessor *proc,
			     PlotAxis *yAxis);

	// Show only the curves whose output DataKey is in `keys`; hide the rest.
	// Pass an empty list to hide all.
	void setVisibleKeys(const QList<scopy::acq::DataKey> &keys);

	// Push a per-sample x-value lookup into every registered AnnotationCurve.
	// Pass an empty vector to revert to sample-index mode (legacy behavior).
	void setSampleXValues(const QVector<double> &xValues);

	void clear();

private Q_SLOTS:
	void onCycleProduced(scopy::acq::DataKey outKey);

private:
	QPointer<PlotWidget>            m_plot;
	QPointer<scopy::acq::DataStore> m_store;

	// Keyed by the decoder's output DataKey.
	QHash<scopy::acq::DataKey, AnnotationCurve *> m_curves;
};

} // namespace adc
} // namespace scopy

#endif
