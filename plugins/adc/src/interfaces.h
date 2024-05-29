#ifndef INTERFACES_H
#define INTERFACES_H
#include "scopy-adcplugin_export.h"
#include <QString>
#include "measurementcontroller.h"

namespace scopy::adc {

class SCOPY_ADCPLUGIN_EXPORT MeasurementProvider {
public:
	virtual MeasureManagerInterface *getMeasureManager() = 0;
};

class SCOPY_ADCPLUGIN_EXPORT SnapshotProvider {
public:
	typedef struct
	{
		std::vector<float> x;
		std::vector<float> y;
		QString name;
	} SnapshotRecipe;

	virtual void addNewSnapshot(SnapshotRecipe) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT SampleRateProvider {
public:
	virtual bool sampleRateAvailable() = 0;
	virtual double sampleRate() = 0;
};


}

#endif // INTERFACES_H
