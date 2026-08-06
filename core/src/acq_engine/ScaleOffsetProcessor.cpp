#include "ScaleOffsetProcessor.h"
#include "ScaleOffsetProcessorWidget.h"

#include "DataStore.h"

#include <QWidget>

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

QList<DataKey> ScaleOffsetProcessor::outputKeys() const
{
	QList<DataKey> result;
	result.reserve(m_channels.size());
	for(const ChannelConfig *cfg : m_channels)
		result.append(cfg->outputKey);
	return result;
}

void ScaleOffsetProcessor::process(DataStore *store)
{
	for(ChannelConfig *cfg : m_channels) {
		const auto src = store->latestAs<QVector<float>>(cfg->inputKey);
		if(!src)
			continue;

		const float scale  = cfg->scale.load(std::memory_order_relaxed);
		const float offset = cfg->offset.load(std::memory_order_relaxed);

		QVector<float> out(src->size());
		for(int i = 0; i < src->size(); ++i)
			out[i] = scale * (*src)[i] + offset;

		store->write(cfg->outputKey, std::move(out));
	}
}

QWidget *ScaleOffsetProcessor::createSettingsWidget(QWidget *parent)
{
	return withBaseSettings(new ScaleOffsetProcessorWidget(this), parent);
}

} // namespace acq
} // namespace scopy
