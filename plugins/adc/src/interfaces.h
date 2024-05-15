#ifndef INTERFACES_H
#define INTERFACES_H
#include "scopy-adcplugin_export.h"
#include <QString>
#include "measurementcontroller.h"
#include "gr-util/grsignalpath.h"

namespace scopy::adc {

class SCOPY_ADCPLUGIN_EXPORT SampleRateUser {
public:
	virtual void setSampleRate(double) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT RollingModeUser {
public:
	virtual void setRollingMode(bool) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT PlotSizeUser {
public:
	virtual void setPlotSize(uint32_t bufferSize) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT BufferSizeUser {
public:
	virtual void setBufferSize(uint32_t bufferSize) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT SingleYModeUser {
public:
	virtual void setSingleYMode(bool) = 0;
};

class SCOPY_ADCPLUGIN_EXPORT XMinMaxUser {
public:
	virtual void setXMinMax(double, double) = 0;
};

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

class SCOPY_ADCPLUGIN_EXPORT XDataModeUser {
public:
	virtual void setXDataMode(struct XDataMode xDataMode) = 0;
};


struct XDataMode{
	enum {TIME, CONST, GR} mode;
	QString grSignalPathName;
	const float *data;
};

class SCOPY_ADCPLUGIN_EXPORT GRSignalPathProvider {
public:
	virtual	grutil::GRSignalPath *signalPath() const = 0;
};


}

#endif // INTERFACES_H
