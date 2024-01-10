#ifndef SCOPY_COMBOGUISTRATEGY_H
#define SCOPY_COMBOGUISTRATEGY_H

#include <QWidget>
#include <iio.h>
#include <gui/widgets/menucombo.h>
#include "guistrategy/guistrategyinterface.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT ComboAttrUi : public QWidget, public AttrUiStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::AttrUiStrategyInterface)
public:
	/**
	 * @brief This contain a MenuComboWidget that takes the options for the combo from recipe->linkedAttributeValue.
	 * */
	explicit ComboAttrUi(IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);
	~ComboAttrUi();

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
	MenuCombo *m_comboWidget;
};
} // namespace scopy

#endif // SCOPY_COMBOGUISTRATEGY_H
