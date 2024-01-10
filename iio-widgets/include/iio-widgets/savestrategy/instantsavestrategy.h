#ifndef SCOPY_INSTANTSAVESTRATEGY_H
#define SCOPY_INSTANTSAVESTRATEGY_H

#include "iiowidgetdata.h"

#include <QWidget>
#include <QTimer>
#include <QPushButton>

#include "gui/widgets/smallprogressbar.h"
#include "savestrategy/savestrategyinterface.h"
#include <iio.h>
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT InstantSaveStrategy : public QWidget, public SaveStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::SaveStrategyInterface)
public:
	explicit InstantSaveStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);

	/**
	 * @overload SaveStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() override;

Q_SIGNALS:
	void saveData(QString data);

public Q_SLOTS:
	void receiveData(QString data) override;

private:
	IIOWidgetFactoryRecipe m_recipe;
};
} // namespace scopy

#endif // SCOPY_INSTANTSAVESTRATEGY_H
