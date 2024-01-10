#ifndef SCOPY_RANGEGUISTRATEGY_H
#define SCOPY_RANGEGUISTRATEGY_H

#include <QWidget>
#include <iio.h>
#include <gui/spinbox_a.hpp>
#include "guistrategy/guistrategyinterface.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT RangeAttrUi : public QWidget, public AttrUiStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::AttrUiStrategyInterface)
public:
	/**
	 * @brief This contain a PositionSpinButton that takes the 3 values from recipe->linkedAttributeValue. The
	 * string from recipe->linkedAttributeValue should look like "[begin step end]" where "begin", "step" and "end"
	 * will be converted to double.
	 * */
	explicit RangeAttrUi(IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);
	~RangeAttrUi();

	/**
	 * @overload AttrUiStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() final;

public Q_SLOTS:
	void receiveData(QString currentData, QString optionalData) override;

Q_SIGNALS:
	void emitData(QString data);
	void requestData();

private:
	QWidget *m_ui;
	PositionSpinButton *m_positionSpinButton;
};
} // namespace scopy

#endif // SCOPY_RANGEGUISTRATEGY_H
