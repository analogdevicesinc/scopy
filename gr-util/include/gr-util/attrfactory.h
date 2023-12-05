#ifndef SCOPY_ATTRFACTORY_H
#define SCOPY_ATTRFACTORY_H

#include <QWidget>
#include <QLoggingCategory>
#include <QMap>
#include <QLabel>
#include <QTimer>
#include <iio.h>
#include "attrdata.h"
#include "attrwidget.h"
#include <scopy-gr-util_export.h>

namespace scopy {
class SCOPY_GR_UTIL_EXPORT AttrFactory : public QWidget
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

	explicit AttrFactory(QWidget *parent = nullptr);
	~AttrFactory();

	AttrWidget *buildSingle(uint32_t hint = AutoHint, AttributeFactoryRecipe recipe = {});
	QList<AttrWidget *> buildAllAttrsForChannel(struct iio_channel *channel);
	QList<AttrWidget *> buildAllAttrsForDevice(struct iio_device *dev);

private:
	struct iio_channel *m_channel;
};
} // namespace scopy

#endif // SCOPY_ATTRFACTORY_H
