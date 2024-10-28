/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef SCOPY_IIOSTANDARDITEM_H
#define SCOPY_IIOSTANDARDITEM_H

#include <QWidget>
#include <QStandardItem>
#include <QDateTime>
#include <iio.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/iiowidget.h>

namespace scopy::debugger {
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
	QString formatExplanation();
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
	void emitLog(QDateTime timestamp, bool isRead, QString path, QString oldValue, QString newValue,
		     int returnCode);

protected Q_SLOTS:
	void forwardLog(QDateTime timestamp, QString data, QString dataOptions, int returnCode, bool isReadOp);

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
	QString m_formatExplanations;
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
} // namespace scopy::debugger

#endif // SCOPY_IIOSTANDARDITEM_H
