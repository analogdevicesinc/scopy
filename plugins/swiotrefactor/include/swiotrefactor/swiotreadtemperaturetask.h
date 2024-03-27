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

#ifndef SCOPY_SWIOTREADTEMPERATURETASK_H
#define SCOPY_SWIOTREADTEMPERATURETASK_H

#include <iio.h>

#include <QThread>

#include <iioutil/command.h>
#include <iioutil/connection.h>

namespace scopy::swiotrefactor {
class SwiotReadTemperatureTask : public QThread
{
	Q_OBJECT
public:
	explicit SwiotReadTemperatureTask(QString uri, QObject *parent = nullptr);
	~SwiotReadTemperatureTask();
	void run() override;

Q_SIGNALS:
	void newTemperature(double value);

private Q_SLOTS:
	void readRawCommandFinished(scopy::Command *cmd);
	void readScaleCommandFinished(scopy::Command *cmd);
	void readOffsetCommandFinished(scopy::Command *cmd);

private:
	QString m_uri;
	struct iio_channel *m_channel;
	struct iio_device *m_device;
	Connection *m_conn;
	double m_raw, m_scale, m_offset;
};
} // namespace scopy::swiotrefactor

#endif // SCOPY_SWIOTREADTEMPERATURETASK_H
