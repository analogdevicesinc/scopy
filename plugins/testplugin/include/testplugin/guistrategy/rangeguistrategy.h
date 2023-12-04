#ifndef SCOPY_RANGEGUISTRATEGY_H
#define SCOPY_RANGEGUISTRATEGY_H

#include <QWidget>
#include <iio.h>
#include <gui/spinbox_a.hpp>
#include "guistrategy/guistrategyinterface.h"
#include "attrdata.h"

namespace scopy::attr {
class RangeAttrUi : public AttrUiStrategyInterface
{
	Q_OBJECT
public:
	/**
	 * @brief This contain a PositionSpinButton that takes the 3 values from recipe->linkedAttributeValue. The
	 * string from recipe->linkedAttributeValue should look like "[begin step end]" where "begin", "step" and "end"
	 * will be converted to double.
	 * */
	explicit RangeAttrUi(AttributeFactoryRecipe recipe, QObject *parent = nullptr);
	~RangeAttrUi();

	/**
	 * @overload AttrUiStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() final;

	void receiveData(QString currentData, QString optionalData) override;

private:
	QWidget *m_ui;
	PositionSpinButton *m_positionSpinButton;
};
} // namespace scopy::attr

#endif // SCOPY_RANGEGUISTRATEGY_H
