#ifndef INTERFACES_H
#define INTERFACES_H
#include "scopy-adc_export.h"
#include <QString>
#include "measurementcontroller.h"

namespace scopy::adc {

typedef enum
{
	YMODE_COUNT,
	YMODE_FS,
	YMODE_SCALE
} YMode;

class SCOPY_ADC_EXPORT MeasurementProvider
{
public:
	virtual MeasureManagerInterface *getMeasureManager() = 0;
};

class TimePlotComponent;
typedef struct
{
	std::vector<float> x;
	std::vector<float> y;
	TimePlotComponent *targetPlot;
	QString name;
} SnapshotRecipe;


class SCOPY_ADC_EXPORT SampleRateProvider
{
public:
	virtual bool sampleRateAvailable() = 0;
	virtual double sampleRate() = 0;
};

class SCOPY_ADC_EXPORT ScaleProvider
{
public:
	virtual YMode ymode() const = 0;
	virtual void setYMode(YMode newYmode) = 0;
	virtual bool scaleAvailable() const = 0;
	virtual bool yLock() const = 0;
	virtual double yMin() const = 0;
	virtual double yMax() const = 0;
};


class SCOPY_ADC_EXPORT MeasurementPanelInterface
{
public:
	virtual MeasurementsPanel *measurePanel() const = 0;
	virtual StatsPanel *statsPanel() const = 0;
	virtual void enableMeasurementPanel(bool) = 0;
	virtual void enableStatsPanel(bool) = 0;
};

} // namespace scopy::adc

#endif // INTERFACES_H
