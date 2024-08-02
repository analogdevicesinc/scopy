#ifndef ROLLINGSTRATEGY_H
#define ROLLINGSTRATEGY_H

#include <plottingstrategy.h>

namespace scopy::pqm {
class RollingStrategy : public PlottingStrategy
{
public:
	RollingStrategy(int samplingFreq);
	~RollingStrategy();

	QMap<QString, QVector<double>> processSamples(QMap<QString, QVector<double>> samples) override;
};
} // namespace scopy::pqm

#endif // ROLLINGSTRATEGY_H
