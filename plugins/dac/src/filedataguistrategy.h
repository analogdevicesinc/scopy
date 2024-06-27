#ifndef FILEDATAGUISTRATEGY_H
#define FILEDATAGUISTRATEGY_H

#include <QWidget>
#include "dataguistrategyinterface.h"
#include "databufferrecipe.h"
#include "scopy-dac_export.h"
#include "dac_logging_categories.h"

namespace scopy {
class MenuOnOffSwitch;
class TitleSpinBox;
namespace dac {
class SCOPY_DAC_EXPORT FileDataGuiStrategy : public QObject, public DataGuiStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::dac::DataGuiStrategyInterface)
public:
	explicit FileDataGuiStrategy(QWidget *parent = nullptr);
	~FileDataGuiStrategy() {}

	QWidget *ui() override;
	void init() override;

Q_SIGNALS:
	void recipeUpdated(DataBufferRecipe) override;

private:
	QWidget *m_ui;
	MenuOnOffSwitch *m_scaled;
	TitleSpinBox *m_scaleSpin;
};
} // namespace dac
} // namespace scopy
#endif // FILEDATAGUISTRATEGY_H
