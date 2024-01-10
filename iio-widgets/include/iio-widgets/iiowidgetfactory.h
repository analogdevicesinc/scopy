#ifndef SCOPY_IIOWIDGETFACTORY_H
#define SCOPY_IIOWIDGETFACTORY_H

#include <QWidget>
#include <QLoggingCategory>
#include <QMap>
#include <QLabel>
#include <QTimer>
#include <iio.h>
#include "iiowidgetdata.h"
#include "iiowidget.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT IIOWidgetFactory : public QWidget
{
	Q_OBJECT
public:
	enum // Attribute factory hint, might have to move to smth like scopy::AFH
	{
		AutoHint = 0x00, // auto hint is 0x0, might change to NoHint

		TimeSave = 0x000001,
		InstantSave = 0x000002,
		ExternalSave = 0x000004,

		EditableUi = 0x000008,
		ComboUi = 0x000010,
		SwitchUi = 0x00020,
		RangeUi = 0x000040,

		AttrData = 0x000080,
		TriggerData = 0x000100,
		DeviceAttrData = 0x000200,
		FileDemoData = 0x000400,

		ProgressLineEditUi = 0x000800,
	};

	explicit IIOWidgetFactory(QWidget *parent = nullptr);
	~IIOWidgetFactory();

	IIOWidget *buildSingle(uint32_t hint = AutoHint, IIOWidgetFactoryRecipe recipe = {});
	QList<IIOWidget *> buildAllAttrsForChannel(struct iio_channel *channel);
	QList<IIOWidget *> buildAllAttrsForDevice(struct iio_device *dev);

private:
	struct iio_channel *m_channel;
};
} // namespace scopy

#endif // SCOPY_IIOWIDGETFACTORY_H
