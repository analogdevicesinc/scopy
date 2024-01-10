#ifndef SCOPY_GUISTRATEGYINTERFACE_H
#define SCOPY_GUISTRATEGYINTERFACE_H

#include <QWidget>
#include <QLoggingCategory>
#include <iio.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/customSwitch.h>
#include <gui/spinbox_a.hpp>
#include <gui/widgets/menucombo.h>
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT AttrUiStrategyInterface
{
public:
	~AttrUiStrategyInterface() = default;

	/**
	 * @brief This should implement the main method for displaying the value of an attribute and also editing data
	 * from that attribute.
	 * */
	virtual QWidget *ui() = 0;

	/**
	 * @brief Checks if the class implemented by this interface has the data it requires to properly function.
	 * @return true if the data provided is enough for the class to work properly.
	 * */
	virtual bool isValid() = 0;

public Q_SLOTS:
	/**
	 * @brief Reads the data from the attributes that are set in the recipe if available.
	 * */
	virtual void receiveData(QString currentData, QString optionalData) = 0;

Q_SIGNALS:
	/**
	 * @brief This will be the signal that the user changed the data, it should be caught by an external function
	 * and set accordingly.
	 * @warning Do not override this.
	 * */
	virtual void emitData(QString data) = 0;

	// sends a request to the data handler to be able to complete the display
	virtual void requestData() = 0;

protected:
	IIOWidgetFactoryRecipe m_recipe;
};
} // namespace scopy

Q_DECLARE_INTERFACE(scopy::AttrUiStrategyInterface, "scopy::AttrUiStrategyInterface")
#endif // SCOPY_GUISTRATEGYINTERFACE_H
