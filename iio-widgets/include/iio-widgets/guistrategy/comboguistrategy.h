#ifndef SCOPY_COMBOGUISTRATEGY_H
#define SCOPY_COMBOGUISTRATEGY_H

#include <QWidget>
#include <iio.h>
#include <gui/widgets/menucombo.h>
#include "guistrategy/guistrategyinterface.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy::attr {
class SCOPY_IIO_WIDGETS_EXPORT ComboAttrUi : public AttrUiStrategyInterface
{
	Q_OBJECT
public:
	/**
	 * @brief This contain a MenuComboWidget that takes the options for the combo from recipe->linkedAttributeValue.
	 * */
	explicit ComboAttrUi(IIOWidgetFactoryRecipe recipe, QObject *parent = nullptr);
	~ComboAttrUi();

	/**
	 * @overload AttrUiStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() final;

	void receiveData(QString currentData, QString optionalData) override;

private:
	QWidget *m_ui;
	MenuCombo *m_comboWidget;
};
} // namespace scopy::attr

#endif // SCOPY_COMBOGUISTRATEGY_H
