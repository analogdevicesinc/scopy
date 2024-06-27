#ifndef DATAGUISTRATEGYINTERFACE_H
#define DATAGUISTRATEGYINTERFACE_H

#include "scopy-dac_export.h"
#include "databufferrecipe.h"
#include <QWidget>

namespace scopy {
namespace dac {
class SCOPY_DAC_EXPORT DataGuiStrategyInterface
{
public:
	virtual ~DataGuiStrategyInterface() = default;
	virtual QWidget *ui() = 0;
	virtual void init() = 0;

Q_SIGNALS:
	virtual void recipeUpdated(DataBufferRecipe) = 0;

protected:
	DataBufferRecipe m_recipe;
};
} // namespace dac
} // namespace scopy
Q_DECLARE_INTERFACE(scopy::dac::DataGuiStrategyInterface, "scopy::DataGuiStrategyInterface")
#endif // DATAGUISTRATEGYINTERFACE_H
