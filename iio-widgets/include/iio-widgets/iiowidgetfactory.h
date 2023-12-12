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

		// save hint mask is 0x____nn
		TimeSave = 0x000001,
		InstantSave = 0x000002,
		ExternalSave = 0x000004,

		// ui hint mask is 0x__nn__
		EditableUi = 0x000100,
		ComboUi = 0x000200,
		SwitchUi = 0x00400,
		RangeUi = 0x000800,

		// data hint mask is 0xnn____
		AttrData = 0x010000,
		TriggerData = 0x020000,
		DeviceAttrData = 0x040000,
		FileDemoData = 0x080000,
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
