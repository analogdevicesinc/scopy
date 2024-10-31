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

#ifndef SWIOTPINGTASK_H
#define SWIOTPINGTASK_H

#include <iio.h>

#include <QThread>

#include "command.h"
#include "connection.h"
#include "pingtask.h"

namespace scopy {
class SCOPY_IIOUTIL_EXPORT CmdQPingTask : public PingTask
{
	Q_OBJECT
public:
	CmdQPingTask(Connection *conn, QObject *parent = nullptr);
	CmdQPingTask(Connection *conn, QString pingDevice, QObject *parent = nullptr);
	~CmdQPingTask();
	void run() override;
	bool ping() override;

protected:
	QString m_pingDevice = "";
	Connection *c;

private Q_SLOTS:
	void getTriggerCommandFinished(scopy::Command *cmd);

private:
	const int MS_TO_WAIT = 2000;
};
} // namespace scopy
#endif // SWIOTPINGTASK_H
