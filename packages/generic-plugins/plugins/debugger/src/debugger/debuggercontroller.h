/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEBUGGERCONTROLLER_H
#define DEBUGGERCONTROLLER_H

#include <iio.h>

#include <QDebug>
#include <QObject>
#include <QVector>

namespace scopy {

namespace debugger {
class DebuggerController : public QObject
{
	Q_OBJECT

public:
	explicit DebuggerController(QObject *parent = nullptr);
	~DebuggerController();
	QStringList getDeviceList(void) const;
	QStringList getChannelList(void) const;
	QStringList getAttributeList(void) const;
	QStringList getFileName(void) const;
	QVector<QString> getAttributeVector(void) const;
	struct iio_context *getIioContext(void);
	void setIioContext(struct iio_context *ctx);

	QString readAttribute(const QString &devName, QString &channel, const QString &attribute);
	QString writeAttribute(const QString &devName, QString &channel, const QString &attribute,
			       const QString &value);
	QStringList getAvailableValues(const QString &devName, QString &channel, QString &attribute) const;
	QString getAttributeValue(const QString &devName, const QString &channel, const QString &attribute) const;
	void setAttributeValue(const QString &devName, const QString &channel, const QString &attribute,
			       const QString &value);

Q_SIGNALS:
	void channelsChanged(const QStringList channelList);

public Q_SLOTS:
	void scanDevices(void);
	void scanChannels(const QString &devName);
	void scanChannelAttributes(QString devName, QString &channel);

private:
	struct iio_context *ctx;
	QStringList deviceList;
	QStringList channelList;
	QStringList attributeList;
	QStringList filename;
	bool connected = false;
	QVector<QString> attributeAvailable;
};
} // namespace debugger
} // namespace scopy

#endif // DEBUGGERCONTROLLER_H
