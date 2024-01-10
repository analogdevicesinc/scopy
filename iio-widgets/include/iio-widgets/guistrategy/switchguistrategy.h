#ifndef SCOPY_SWITCHGUISTRATEGY_H
#define SCOPY_SWITCHGUISTRATEGY_H

#include <QWidget>
#include <iio.h>
#include <gui/customSwitch.h>
#include "guistrategy/guistrategyinterface.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT SwitchAttrUi : public QWidget, public AttrUiStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::AttrUiStrategyInterface)
public:
	/**
	 * @brief This contain a CustomSwitch capable of holding no more than 2 values, the ones specified in
	 * recipe->linkedAttributeValue.
	 * */
	explicit SwitchAttrUi(IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);
	~SwitchAttrUi();

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
	CustomSwitch *m_menuBigSwitch;
	QStringList *m_optionsList;
};
} // namespace scopy

#endif // SCOPY_SWITCHGUISTRATEGY_H
