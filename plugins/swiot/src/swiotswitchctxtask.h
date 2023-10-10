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

#ifndef SWIOTSWITCHCTXTASK_H
#define SWIOTSWITCHCTXTASK_H

#include <QObject>
#include <QThread>

namespace scopy::swiot {
class SwiotSwitchCtxTask : public QThread
{
	Q_OBJECT
public:
	SwiotSwitchCtxTask(QString uri, bool wasRuntime);
	void run() override;

Q_SIGNALS:
	void contextSwitched();

private:
	QString m_uri;
	bool m_wasRuntime;
};
} // namespace scopy::swiot

#endif // SWIOTSWITCHCTXTASK_H
