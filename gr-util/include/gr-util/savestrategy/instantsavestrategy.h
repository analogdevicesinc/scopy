#ifndef SCOPY_INSTANTSAVESTRATEGY_H
#define SCOPY_INSTANTSAVESTRATEGY_H

#include "attrdata.h"

#include <QWidget>
#include <QTimer>
#include <QPushButton>

#include "gui/widgets/smallprogressbar.h"
#include "savestrategy/savestrategyinterface.h"
#include <iio.h>

namespace scopy::attr {
class InstantSaveStrategy : public SaveStrategyInterface
{
	Q_OBJECT
public:
	explicit InstantSaveStrategy(AttributeFactoryRecipe recipe, QObject *parent = nullptr);

	/**
	 * @overload SaveStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() override;

public Q_SLOTS:
	void receiveData(QString data) override;

private:
	AttributeFactoryRecipe m_recipe;
};
} // namespace scopy::attr

#endif // SCOPY_INSTANTSAVESTRATEGY_H
