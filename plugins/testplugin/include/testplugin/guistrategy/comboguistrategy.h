#ifndef SCOPY_COMBOGUISTRATEGY_H
#define SCOPY_COMBOGUISTRATEGY_H

#include <QWidget>
#include <iio.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/customSwitch.h>
#include <gui/spinbox_a.hpp>
#include <gui/widgets/menucombo.h>
#include "guistrategy/guistrategyinterface.h"
#include "attrdata.h"

namespace scopy::attr {
class ComboAttrUi : public AttrUiStrategyInterface
{
	Q_OBJECT
public:
	/**
	 * @brief This contain a MenuComboWidget that takes the options for the combo from recipe->linkedAttributeValue.
	 * */
	explicit ComboAttrUi(AttributeFactoryRecipe recipe, QObject *parent = nullptr);
	~ComboAttrUi();

	/**
	 * @overload AttrUiStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() final;

	void receiveData(QString currentData, QString optionalData) override;

private:
	QWidget *m_ui;
	MenuComboWidget *m_menuComboWidget;
};
} // namespace scopy::attr

#endif // SCOPY_COMBOGUISTRATEGY_H
