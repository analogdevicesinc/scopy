#ifndef SCOPY_IIOMODEL_H
#define SCOPY_IIOMODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <QSet>
#include <iio-widgets/iiowidgetfactory.h>
#include <iio.h>
#include "iiostandarditem.h"

namespace scopy::iiodebugplugin {
class IIOModel : public QObject
{
	Q_OBJECT
public:
	explicit IIOModel(struct iio_context *context, QString uri, QObject *parent = nullptr);

	QStandardItemModel *getModel();
	QSet<QString> getEntries();

private:
	void iioTreeSetup();
	void setupCtx();
	void generateCtxAttributes();
	void setupCurrentDevice();
	void generateDeviceAttributes();
	void setupCurrentChannel();
	void generateChannelAttributes();

	QStandardItemModel *m_model;
	struct iio_context *m_ctx;
	QSet<QString> m_entries;
	QString m_uri;

	// members used in the setup of the iio tree
	IIOStandardItem *m_rootItem;
	IIOStandardItem *m_currentDeviceItem;
	IIOStandardItem *m_currentChannelItem;

	QString m_rootString;
	QString m_currentDeviceName;
	QString m_currentChannelName;

	struct iio_device *m_currentDevice;
	struct iio_channel *m_currentChannel;

	int m_currentDeviceIndex;
	int m_currentChannelIndex;

	QList<IIOWidget *> m_ctxList;
	QList<IIOWidget *> m_devList;
	QList<IIOWidget *> m_chnlList;
};
} // namespace scopy::iiodebugplugin

#endif // SCOPY_IIOMODEL_H
