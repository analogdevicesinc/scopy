#ifndef SWTRIGGERSTRATEGY_H
#define SWTRIGGERSTRATEGY_H

#include <plottingstrategy.h>

namespace scopy::pqm {

class SwTriggerStrategy : public PlottingStrategy
{
public:
	SwTriggerStrategy(int samplingFreq, QString triggeredBy);
	~SwTriggerStrategy();

	QMap<QString, QVector<double>> processSamples(QMap<QString, QVector<double>> samples) override;

private:
	void zeroCrossing(QMap<QString, QVector<double>> samples);
	bool m_fill;
	QString m_triggeredBy;
};

} // namespace scopy::pqm

#endif // SWTRIGGERSTRATEGY_H
