#include "DecoderOverlay.h"

#include "acq_engine/DataKey.h"
#include "acq_engine/DataStore.h"
#include "acq_engine/ExternalDecoderProcessor.h"
#include "acq_engine/SampleBuffer.h"
#include "gui/plotwidget.h"

#include <iostream>
#include <variant>

namespace scopy {
namespace adc {

DecoderOverlay::DecoderOverlay(PlotWidget *plot, scopy::acq::DataStore *store,
			       QObject *parent)
	: QObject(parent)
	, m_plot(plot)
	, m_store(store)
{}

DecoderOverlay::~DecoderOverlay() = default;

void DecoderOverlay::registerDecoder(scopy::acq::ExternalDecoderProcessor *proc)
{
	if(!proc) return;
	connect(proc, &scopy::acq::ExternalDecoderProcessor::cycleProduced,
		this, &DecoderOverlay::onCycleProduced,
		Qt::QueuedConnection);
}

void DecoderOverlay::onCycleProduced(scopy::acq::DataKey outKey)
{
	if(!m_store) return;
	const scopy::acq::SampleBuffer buf = m_store->read(outKey);
	if(buf.empty()) return;

	const scopy::acq::SampleVariant &v = buf.sample(0);
	if(!std::holds_alternative<QVector<scopy::acq::Annotation>>(v)) return;

	const auto &anns = std::get<QVector<scopy::acq::Annotation>>(v);
	for(const auto &a : anns) {
		std::cout << "[annotation] " << a.decoder.toStdString()
			  << " " << a.startSample << "-" << a.endSample
			  << " " << a.klass.toStdString()
			  << " : " << a.text.toStdString()
			  << std::endl;
	}
}

} // namespace adc
} // namespace scopy
