#ifndef SCOPY_ATTRWIDGET_H
#define SCOPY_ATTRWIDGET_H

#include <QWidget>
#include "attrdata.h"
#include "utils.h"
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menusectionwidget.h>
#include "guistrategy/guistrategyinterface.h"
#include "savestrategy/savestrategyinterface.h"
#include "datastrategy/datastrategyinterface.h"

namespace scopy {
namespace attr {
class AttrUiStrategyInterface;
class SaveStrategyInterface;
class DataStrategyInterface;
} // namespace attr

class AttrWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	AttrWidget(attr::AttrUiStrategyInterface *uiStrategy, attr::SaveStrategyInterface *saveStrategy,
		   attr::DataStrategyInterface *dataStrategy, QWidget *parent = nullptr);

	attr::SaveStrategyInterface *getSaveStrategy();
	attr::AttrUiStrategyInterface *getUiStrategy();
	attr::DataStrategyInterface *getDataStrategy();

	/**
	 * @brief Returns the recipe that this widget is based on. This is optional, currently serves as a way to pass
	 * information about the current widget.
	 * @return AttributeFactoryRecipe
	 * */
	AttributeFactoryRecipe getRecipe();

	/**
	 * @brief Sets the recipe that this widget is based on. This is optional, currently serves as a way to pass
	 * information about the current widget.
	 * @return void
	 * */
	void setRecipe(AttributeFactoryRecipe recipe);

Q_SIGNALS:
	/**
	 * @brief 0 - busy, 1 - correct, 2 error
	 * */
	void currentStateChanged(int currentState, QString explanation = "");

protected:
	attr::SaveStrategyInterface *m_saveStrategy;
	attr::AttrUiStrategyInterface *m_uiStrategy;
	attr::DataStrategyInterface *m_dataStrategy;
	AttributeFactoryRecipe m_recipe;
};
} // namespace scopy

#endif // SCOPY_ATTRWIDGET_H
