#ifndef PLOTTINGSTRATEGY_H
#define PLOTTINGSTRATEGY_H

#include <QString>
#include <QMap>

namespace scopy::pqm {

class PlottingStrategy
{

public:
	PlottingStrategy(int samplingFreq) { m_samplingFreq = samplingFreq; }
	virtual ~PlottingStrategy() { m_samples.clear(); }

	virtual QMap<QString, QVector<double>> processSamples(QMap<QString, QVector<double>> samples) = 0;
	bool dataReady() const;
	void setSamplingFreq(int newSamplingFreq);
	void clearSamples();

protected:
	int m_samplingFreq;
	bool m_dataReady = false;
	QMap<QString, QVector<double>> m_samples;
};

inline bool PlottingStrategy::dataReady() const { return m_dataReady; }

inline void PlottingStrategy::setSamplingFreq(int newSamplingFreq) { m_samplingFreq = newSamplingFreq; }

inline void PlottingStrategy::clearSamples() { m_samples.clear(); }

} // namespace scopy::pqm

#endif // PLOTTINGSTRATEGY_H
