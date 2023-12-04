#ifndef SCOPY_EXTERNALSAVESTRATEGY_H
#define SCOPY_EXTERNALSAVESTRATEGY_H

#include "attrdata.h"

#include <QWidget>
#include <QTimer>
#include <QPushButton>

#include "gui/widgets/smallprogressbar.h"
#include "savestrategy/savestrategyinterface.h"
#include <iio.h>

namespace scopy::attr {
class ExternalTriggerSaveStrategy : public SaveStrategyInterface
{
	Q_OBJECT
public:
	explicit ExternalTriggerSaveStrategy(AttributeFactoryRecipe recipe, QObject *parent = nullptr);
	~ExternalTriggerSaveStrategy();

	/**
	 * @overload SaveStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() override;

public Q_SLOTS:
	void receiveData(QString data) override;
	void writeData();

private:
	AttributeFactoryRecipe m_recipe;
	QPushButton *m_ui;
	QString m_data;
	bool m_dataReceived;
};
} // namespace scopy::attr

#endif // SCOPY_EXTERNALSAVESTRATEGY_H
