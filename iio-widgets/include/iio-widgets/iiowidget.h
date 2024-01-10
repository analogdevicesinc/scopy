#ifndef SCOPY_IIOWIDGET_H
#define SCOPY_IIOWIDGET_H

#include <QWidget>
#include "iiowidgetdata.h"
#include "utils.h"
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menusectionwidget.h>
#include "guistrategy/guistrategyinterface.h"
#include "savestrategy/savestrategyinterface.h"
#include "datastrategy/datastrategyinterface.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class AttrUiStrategyInterface;
class SaveStrategyInterface;
class DataStrategyInterface;

class SCOPY_IIO_WIDGETS_EXPORT IIOWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	IIOWidget(AttrUiStrategyInterface *uiStrategy, SaveStrategyInterface *saveStrategy,
		  DataStrategyInterface *dataStrategy, QWidget *parent = nullptr);

	SaveStrategyInterface *getSaveStrategy();
	AttrUiStrategyInterface *getUiStrategy();
	DataStrategyInterface *getDataStrategy();

	/**
	 * @brief Returns the recipe that this widget is based on. This is optional, currently serves as a way to pass
	 * information about the current widget.
	 * @return AttributeFactoryRecipe
	 * */
	IIOWidgetFactoryRecipe getRecipe();

	/**
	 * @brief Sets the recipe that this widget is based on. This is optional, currently serves as a way to pass
	 * information about the current widget.
	 * @return void
	 * */
	void setRecipe(IIOWidgetFactoryRecipe recipe);

	typedef enum
	{
		Busy,
		Correct,
		Error
	} State;

Q_SIGNALS:
	/**
	 * @brief 0 - busy, 1 - correct, 2 error
	 * */
	void currentStateChanged(State currentState, QString explanation = "");

protected Q_SLOTS:
	void saveData(QString data);
	void emitDataStatus(int status);

protected:
	SaveStrategyInterface *m_saveStrategy;
	AttrUiStrategyInterface *m_uiStrategy;
	DataStrategyInterface *m_dataStrategy;
	IIOWidgetFactoryRecipe m_recipe;
};
} // namespace scopy

#endif // SCOPY_IIOWIDGET_H
