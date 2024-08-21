#ifndef INTERFACES_H
#define INTERFACES_H
#include "markercontroller.h"
#include "plotwidget.h"
#include "scopy-adc_export.h"
#include <QString>
#include "measurementcontroller.h"
#include "menuwidget.h"
#include <iioutil/iiounits.h>

namespace scopy::adc {
using namespace scopy;
using namespace scopy::gui;

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

class SCOPY_ADC_EXPORT Menu
{
public:
	virtual MenuWidget *menu() = 0;
};

class SCOPY_ADC_EXPORT FFTChannel
{
public:
	virtual void setPowerOffset(double) = 0;
	virtual void setWindow(int) = 0;
	virtual void setWindowCorrection(bool) = 0;
};

class SCOPY_ADC_EXPORT SampleRateProvider
{
public:
	virtual bool sampleRateAvailable() = 0;
	virtual double sampleRate() = 0;
};

class SCOPY_ADC_EXPORT DataProcessor
{
public:
	virtual void onNewData(const float *xData, const float *yData, size_t size, bool copy) = 0;
};

class SCOPY_ADC_EXPORT SamplingInfoComponent
{
public:
	virtual SamplingInfo samplingInfo() = 0;
	virtual void setSamplingInfo(SamplingInfo p) = 0;
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
	virtual IIOUnit unit() const = 0;
};

class SCOPY_ADC_EXPORT MeasurementPanelInterface
{
public:
	virtual MeasurementsPanel *measurePanel() const = 0;
	virtual StatsPanel *statsPanel() const = 0;
	virtual MarkerPanel *markerPanel() const = 0;
	virtual void enableMeasurementPanel(bool) = 0;
	virtual void enableStatsPanel(bool) = 0;
	virtual void enableMarkerPanel(bool) = 0;
};

} // namespace scopy::adc

#endif // INTERFACES_H
