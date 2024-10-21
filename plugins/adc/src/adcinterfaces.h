#ifndef ADC_INTERFACES_H
#define ADC_INTERFACES_H

#include "scopy-adc_export.h"
#include <QString>
#include "measurementcontroller.h"
#include <gr-util/grsignalpath.h>
#include <gui/plotmarkercontroller.h>
#include <gui/interfaces.h>

namespace scopy {

using namespace grutil;

namespace adc {

class SCOPY_ADC_EXPORT GRChannel : public DataProcessor
{
public:
	virtual GRSignalPath *sigpath() = 0;
};

class TimePlotComponent;
typedef struct
{
	std::vector<float> x;
	std::vector<float> y;
	TimePlotComponent *targetPlot;
	QString name;
} SnapshotRecipe;

class SCOPY_ADC_EXPORT MeasurementProvider
{
public:
	virtual MeasureManagerInterface *getMeasureManager() = 0;
};

} // namespace adc
} // namespace scopy

#endif // ADC_INTERFACES_H
