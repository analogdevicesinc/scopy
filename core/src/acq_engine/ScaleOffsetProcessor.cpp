#include "ScaleOffsetProcessor.h"
#include "ScaleOffsetProcessorWidget.h"

#include "DataStore.h"

#include <variant>

namespace scopy {
namespace acq {

ScaleOffsetProcessor::ScaleOffsetProcessor(const QString &name, QObject *parent)
	: ProcessorBlock(name, parent)
{}

ScaleOffsetProcessor::~ScaleOffsetProcessor()
{
	qDeleteAll(m_channels);
}

ScaleOffsetProcessor::ChannelConfig *ScaleOffsetProcessor::addChannel(const DataKey &inputKey,
								       const DataKey &outputKey,
								       const QString &label)
{
	auto *cfg = new ChannelConfig(inputKey, outputKey, label.isEmpty() ? inputKey.key : label);
	m_channels.append(cfg);
	m_watchedKeys.append(inputKey);
	return cfg;
}

void ScaleOffsetProcessor::process(DataStore *store)
{
	for(ChannelConfig *cfg : m_channels) {
		const SampleBuffer buf = store->read(cfg->inputKey);
		if(buf.empty())
			continue;

		const auto &v = buf.sample(0);
		if(!std::holds_alternative<QVector<float>>(v))
			continue;

		const QVector<float> &src    = std::get<QVector<float>>(v);
		const float           scale  = cfg->scale.load(std::memory_order_relaxed);
		const float           offset = cfg->offset.load(std::memory_order_relaxed);

		QVector<float> out(src.size());
		for(int i = 0; i < src.size(); ++i)
			out[i] = scale * src[i] + offset;

		store->write(cfg->outputKey, std::move(out));
	}
}

QWidget *ScaleOffsetProcessor::createSettingsWidget(QWidget *parent)
{
	return new ScaleOffsetProcessorWidget(this, parent);
}

} // namespace acq
} // namespace scopy
