#ifndef SCOPY_IIOSTANDARDITEM_H
#define SCOPY_IIOSTANDARDITEM_H

#include <QWidget>
#include <QStandardItem>
#include <QDateTime>
#include <iio.h>
#include <iio-widgets/iiowidgetfactory.h>
#include <iio-widgets/iiowidget.h>

namespace scopy::iiodebugplugin {
class IIOStandardItem : public QObject, public QStandardItem
{
	Q_OBJECT
public:
	enum Type
	{
		Context,
		Device,
		Trigger,
		Channel,
		ContextAttribute,
		DeviceAttribute,
		ChannelAttribute,
	};

	explicit IIOStandardItem(QList<IIOWidget *> widgets, QString name, QString path, IIOStandardItem::Type type);
	explicit IIOStandardItem(QList<IIOWidget *> widgets, QString name, QString id, QString path,
				 IIOStandardItem::Type type);

	~IIOStandardItem() override;

	/**
	 * @brief Sets the iio_device for this IIOStandardItem. Upon setting it, some libiio calls will be made to query
	 * the device
	 * @param struct iio_device *
	 */
	void setDevice(struct iio_device *device);

	/**
	 * @brief Sets the iio_channel for this IIOStandardItem. Upon setting it, some libiio calls will be made to
	 * query the channel.
	 * @param struct iio_channel *
	 */
	void setChannel(struct iio_channel *channel);

	/**
	 * @brief getIIOWidgets
	 * @return The list of IIOWidgets that is makes up this item.
	 */
	QList<IIOWidget *> getIIOWidgets();

	QStringList details();
	QString id();
	QString name();
	QString path();
	QString format();
	QString trigger();
	QString triggerStatus();
	IIOStandardItem::Type type();

	/**
	 * @brief Specific to Channel types, returns the index in the channel
	 */
	int index();

	bool isScanElement();
	bool isOutput();
	bool isEnabled();

	bool isBufferCapable();

	/**
	 * @brief Only a device type can be buffer capable, set this to true only
	 * if there is at least 1 channel in this device that is a scan_element.
	 * @param isBufferCapable Set to true if the device is buffer capable, false if it is not.
	 */
	void setBufferCapable(bool isBufferCapable);

	bool isWatched();
	void setWatched(bool isWatched);

	QString typeString();

Q_SIGNALS:
	void emitLog(QDateTime *timestamp, bool isRead, QString path, QString oldValue, QString newValue);

protected Q_SLOTS:
	void emitWriteLog(QString oldValue, QString newValue);
	void emitReadLog(QString data, QString dataOptions);

private:
	void connectLog();
	void buildDetails();
	void generateToolTip();

	void extractDataFromDevice();
	void extractDataFromChannel();

	struct iio_device *m_device;
	struct iio_channel *m_channel;
	QList<IIOWidget *> m_iioWidgets;
	QStringList m_details;
	QString m_name;
	QString m_id;
	QString m_path;
	QString m_format;
	QString m_triggerName;
	QString m_triggerStatus;
	IIOStandardItem::Type m_type;
	bool m_isWatched;
	bool m_isScanElement;
	bool m_isOutput;
	bool m_isEnabled;
	bool m_isBufferCapable;
	int m_index;
};
} // namespace scopy::iiodebugplugin

#endif // SCOPY_IIOSTANDARDITEM_H
