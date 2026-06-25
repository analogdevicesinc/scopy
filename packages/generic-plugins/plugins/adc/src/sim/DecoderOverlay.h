#ifndef SCOPY_ADC_DECODEROVERLAY_H
#define SCOPY_ADC_DECODEROVERLAY_H

#include <QObject>
#include <QPointer>

#include "acq_engine/DataKey.h"

namespace scopy {

class PlotWidget;

namespace acq {
class ExternalDecoderProcessor;
class DataStore;
} // namespace acq

namespace adc {

// Subscribes to ExternalDecoderProcessor::cycleProduced, then reads the
// produced annotations out of the DataStore and prints them. This is the
// temporary debug-only consumer; plot drawing will be reintroduced later.
class DecoderOverlay : public QObject
{
	Q_OBJECT
public:
	DecoderOverlay(PlotWidget *plot, scopy::acq::DataStore *store,
		       QObject *parent = nullptr);
	~DecoderOverlay() override;

	void registerDecoder(scopy::acq::ExternalDecoderProcessor *proc);

	void clear() {}

private Q_SLOTS:
	void onCycleProduced(scopy::acq::DataKey outKey);

private:
	QPointer<PlotWidget>            m_plot;
	QPointer<scopy::acq::DataStore> m_store;
};

} // namespace adc
} // namespace scopy

#endif
