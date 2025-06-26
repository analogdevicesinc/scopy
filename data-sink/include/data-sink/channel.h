#ifndef CHANNEL_H
#define CHANNEL_H

#include "scopy-data-sink_export.h"
#include "timechannelsigpath.h"
/**
 * Base channel interface
 */
namespace scopy::datasink {
class SCOPY_DATA_SINK_EXPORT Channel
{
public:
	virtual ~Channel() = default;
	virtual void onNewData(const float *xData, const float *yData, size_t size, bool copy) = 0;
	virtual SignalPath* sigpath() = 0;
};
}

#endif // CHANNEL_H
