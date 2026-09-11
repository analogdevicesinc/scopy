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

#ifndef SCOPY_EXTERNALPSREADER_H
#define SCOPY_EXTERNALPSREADER_H

#include <QObject>
#include <qcoro/qcorotask.h>
#include <component/controller.h>

namespace scopy {
namespace component {
class Device;
}
namespace swiot {
class ExternalPsReader : public QObject
{
	Q_OBJECT
public:
	explicit ExternalPsReader(QString uri, QString attr, QObject *parent = nullptr);
	~ExternalPsReader();

	QCoro::Task<void> readPowerSupply();

Q_SIGNALS:
	void hasConnectedPowerSupply(bool ps);

private:
	QString m_uri;
	QString m_attribute;
	component::ContextHandle m_context;
	component::Device *m_swiot;
};
} // namespace swiot
} // namespace scopy

#endif // SCOPY_EXTERNALPSREADER_H
