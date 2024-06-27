#ifndef DATABUFFERSTRATEGYINTERFACE_H
#define DATABUFFERSTRATEGYINTERFACE_H

#include "scopy-dac_export.h"
#include "databufferrecipe.h"
#include <QObject>
#include <QVector>

namespace scopy {
namespace dac {
class SCOPY_DAC_EXPORT DataBufferStrategyInterface
{
public:
	virtual ~DataBufferStrategyInterface() = default;

	virtual QVector<QVector<short>> data() = 0;
public Q_SLOTS:
	virtual void recipeUpdated(DataBufferRecipe) = 0;
	virtual void loadData() = 0;
Q_SIGNALS:
	virtual void loadFinished() = 0;
	virtual void loadFailed() = 0;
	virtual void dataUpdated() = 0;
};
} // namespace dac
} // namespace scopy
Q_DECLARE_INTERFACE(scopy::dac::DataBufferStrategyInterface, "scopy::DataBufferStrategyInterface")
#endif // DATABUFFERSTRATEGYINTERFACE_H
