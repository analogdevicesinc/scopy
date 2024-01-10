#ifndef SCOPY_EDITABLEGUISTRATEGY_H
#define SCOPY_EDITABLEGUISTRATEGY_H

#include <QWidget>
#include <iio.h>
#include "guistrategy/guistrategyinterface.h"
#include <gui/widgets/menulineedit.h>
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT EditableGuiStrategy : public QWidget, public AttrUiStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::AttrUiStrategyInterface)
public:
	/**
	 * @brief This contain a MenuLineEdit with no validation on what the text can or cannot be set.
	 * */
	explicit EditableGuiStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);
	~EditableGuiStrategy();

	/**
	 * @overload AttrUiStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() override;

public Q_SLOTS:
	void receiveData(QString currentData, QString optionalData) override;

Q_SIGNALS:
	void emitData(QString data);
	void requestData();

private:
	QWidget *m_ui;
	MenuLineEdit *m_lineEdit;
};
} // namespace scopy

#endif // SCOPY_EDITABLEGUISTRATEGY_H
