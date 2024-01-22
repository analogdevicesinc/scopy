#ifndef SCOPY_IIOSTANDARDITEM_H
#define SCOPY_IIOSTANDARDITEM_H

#include <QWidget>
#include <QStandardItem>
#include "iiostandarditemmodel.h"
#include "iiostandarditemtypes.h"
#include "iiostandarditemview.h"

#include <iio.h>
#include <iio-widgets/iiowidgetfactory.h>
#include <iio-widgets/iiowidget.h>

namespace scopy::iiodebugplugin {
class IIOStandardItem : public QStandardItem
{
public:
	// TODO: make more classes, this is not right, but it'll do for now
	explicit IIOStandardItem(struct iio_context *context, const QString &text, IIOStandardItemType type = IIOStandardItemType::Undefined);
	explicit IIOStandardItem(struct iio_device *device, const QString &text, IIOStandardItemType type = IIOStandardItemType::Undefined);
	explicit IIOStandardItem(struct iio_channel *channel, const QString &text, IIOStandardItemType type = IIOStandardItemType::Undefined);
	~IIOStandardItem() override;

	QList<IIOWidget *> getIIOWidgets();

private:
	IIOStandardItemModel *m_itemModel;
	IIOStandardItemView *m_itemView;

	IIOWidgetFactory *m_factory;

	struct iio_context *m_context = nullptr;
	struct iio_device *m_device = nullptr;
	struct iio_channel *m_channel = nullptr;

	QList<IIOWidget *> m_iioWidgets;
};
} // namespace scopy::iiodebugplugin

#endif // SCOPY_IIOSTANDARDITEM_H
