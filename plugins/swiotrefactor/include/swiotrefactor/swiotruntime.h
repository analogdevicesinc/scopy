/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef SWIOTRUNTIME_H
#define SWIOTRUNTIME_H

#include <QObject>
#include <QMap>
#include <iio.h>

#include <iioutil/commandqueue.h>

namespace scopy::swiotrefactor {
#define AD_NAME "ad74413r"
#define AD_TRIGGER_NAME "ad74413r-dev0"

class SwiotRuntime : public QObject
{
	Q_OBJECT
public:
	SwiotRuntime(QString m_uri, QObject *parent = nullptr);
	~SwiotRuntime();

public Q_SLOTS:
	void onBackBtnPressed();
	void writeTriggerDevice();
	void onIsRuntimeCtxChanged(bool isRuntimeCtx);
private Q_SLOTS:
	void setTriggerCommandFinished(scopy::Command *);
Q_SIGNALS:
	void writeModeAttribute(QString mode);

private:
	void createDevicesMap();

private:
	QString m_uri;
	iio_context *m_iioCtx;
	QMap<QString, struct iio_device *> m_iioDevices;
	CommandQueue *m_cmdQueue;
};
} // namespace scopy::swiotrefactor

#endif // SWIOTRUNTIME_H
