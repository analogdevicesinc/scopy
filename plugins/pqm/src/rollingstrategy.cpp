#include <rollingstrategy.h>

using namespace scopy::pqm;

RollingStrategy::RollingStrategy(int samplingFreq)
	: PlottingStrategy(samplingFreq)
{
	m_dataReady = true;
}

RollingStrategy::~RollingStrategy() {}

QMap<QString, QVector<double>> RollingStrategy::processSamples(QMap<QString, QVector<double>> samples)
{
	const QStringList keys = samples.keys();
	for(const auto &key : keys) {
		m_samples[key].append(samples[key]);
		if(m_samples[key].size() > m_samplingFreq) {
			int unnecessarySamples = m_samples[key].size() - m_samplingFreq;
			m_samples[key].erase(m_samples[key].begin(), m_samples[key].begin() + unnecessarySamples);
		}
	}
	return m_samples;
}
