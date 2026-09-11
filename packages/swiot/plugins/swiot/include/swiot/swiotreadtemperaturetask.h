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

#include <QObject>
#include <qcoro/qcorotask.h>
#include <component/controller.h>

namespace scopy {
namespace component {
class Channel;
}
namespace swiot {
class SwiotReadTemperatureTask : public QObject
{
	Q_OBJECT
public:
	explicit SwiotReadTemperatureTask(QString uri, QObject *parent = nullptr);
	~SwiotReadTemperatureTask();

	QCoro::Task<void> readTemperature();

Q_SIGNALS:
	void newTemperature(double value);

private:
	QCoro::Task<void> initScaleOffset();

	QString m_uri;
	component::ContextHandle m_context;
	component::Channel *m_channel;
	double m_scale, m_offset;
	bool m_initialized;
};
} // namespace swiot
} // namespace scopy

#endif // SCOPY_SWIOTREADTEMPERATURETASK_H
