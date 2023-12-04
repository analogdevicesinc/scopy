#ifndef SCOPY_EDITABLEGUISTRATEGY_H
#define SCOPY_EDITABLEGUISTRATEGY_H

#include <QWidget>
#include <QLineEdit>
#include <iio.h>
#include "guistrategy/guistrategyinterface.h"
#include "attrdata.h"

namespace scopy::attr {
class EditableGuiStrategy : public AttrUiStrategyInterface
{
	Q_OBJECT
public:
	/**
	 * @brief This contain a MenuLineEdit with no validation on what the text can or cannot be set.
	 * */
	explicit EditableGuiStrategy(AttributeFactoryRecipe recipe, QObject *parent = nullptr);
	~EditableGuiStrategy();

	/**
	 * @overload AttrUiStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() override;

	void receiveData(QString currentData, QString optionalData) override;

private:
	QWidget *m_ui;
	QLineEdit *m_lineEdit;
};
} // namespace scopy::attr

#endif // SCOPY_EDITABLEGUISTRATEGY_H
