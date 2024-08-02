#include <swtriggerstrategy.h>

using namespace scopy::pqm;

SwTriggerStrategy::SwTriggerStrategy(int samplingFreq, QString triggeredBy)
	: PlottingStrategy(samplingFreq)
	, m_triggeredBy(triggeredBy)
{}

SwTriggerStrategy::~SwTriggerStrategy() {}

QMap<QString, QVector<double>> SwTriggerStrategy::processSamples(QMap<QString, QVector<double>> samples)
{
	if(!m_fill) {
		m_dataReady = false;
		zeroCrossing(samples);
	} else {
		const QStringList keys = samples.keys();
		for(const auto &key : keys) {
			m_samples[key].append(samples[key]);
			if(m_samples[key].size() > m_samplingFreq) {
				m_samples[key].erase(m_samples[key].begin() + m_samplingFreq, m_samples[key].end());
			}
		}
		if(m_samples.first().size() == m_samplingFreq) {
			m_fill = false;
			m_dataReady = true;
		}
	}
	return m_samples;
}

void SwTriggerStrategy::zeroCrossing(QMap<QString, QVector<double>> samples)
{
	QString chnl = (m_triggeredBy.isEmpty()) ? "ua" : m_triggeredBy;
	bool valid = false;

	for(int i = 0; i < samples[chnl].size() - 1; i++) {
		if(samples[chnl][i] <= 0 && samples[chnl][i + 1] >= 0) {
			m_fill = true;
			m_samples.clear();
			const QStringList keys = samples.keys();
			for(const auto &key : keys) {
				samples[key].erase(samples[key].begin(), samples[key].begin() + i);
				m_samples[key].append(samples[key]);
			}
			break;
		}
	}
}
