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

#ifndef SCOPY_IIOMODEL_H
#define SCOPY_IIOMODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <QSet>
#include <iio-widgets/iiowidgetbuilder.h>
#include <iio.h>
#include "iiostandarditem.h"

namespace scopy::debugger {
class IIOModel : public QObject
{
	Q_OBJECT
public:
	explicit IIOModel(struct iio_context *context, QString uri, QWidget *parent = nullptr);

	QStandardItemModel *getModel();
	QSet<QString> getEntries();

Q_SIGNALS:
	void emitLog(QDateTime timestamp, bool isRead, QString path, QString oldValue, QString newValue,
		     int returnCode);

private:
	void iioTreeSetup();
	void setupCtx();
	void generateCtxAttributes();
	void setupCurrentDevice();
	void generateDeviceAttributes();
	void setupCurrentChannel();
	void generateChannelAttributes();

	/**
	 * @brief Creates a new IIOStandardItem and connects it to the emitLog signal.
	 * @param widgets The iio-widgets that will make up this IIOStandardItem.
	 * @param name The name of this IIOStandardItem.
	 * @param id The id of this IIOStandardItem, if not available pass an empty string.
	 * @param path The path from the root of the context to the "name".
	 * @param type The type of the IIOStandardItem.
	 * @return The created IIOStandardItem.
	 */
	IIOStandardItem *createIIOStandardItem(QList<IIOWidget *> widgets, QString name, QString id, QString path,
					       IIOStandardItem::Type type);

	QStandardItemModel *m_model;
	struct iio_context *m_ctx;
	QSet<QString> m_entries;
	QString m_uri;
	QWidget *m_parent;

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
} // namespace scopy::debugger

#endif // SCOPY_IIOMODEL_H
