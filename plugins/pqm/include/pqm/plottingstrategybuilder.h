#ifndef PLOTTINGSTRATEGYBUILDER_H
#define PLOTTINGSTRATEGYBUILDER_H
#include "rollingstrategy.h"
#include "swtriggerstrategy.h"

namespace scopy::pqm {
class PlottingStrategyBuilder
{
public:
	static PlottingStrategy *build(QString mode, int samplingFreq, QString triggeredBy = "")
	{
		if(mode.compare("rolling") == 0) {
			return new RollingStrategy(samplingFreq);
		} else {
			return new SwTriggerStrategy(samplingFreq, triggeredBy);
		}
	}
};
} // namespace scopy::pqm

#endif // PLOTTINGSTRATEGYBUILDER_H
