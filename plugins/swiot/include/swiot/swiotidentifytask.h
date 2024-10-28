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

#ifndef SWIOTIDENTIFYTASK_H
#define SWIOTIDENTIFYTASK_H

#include <QThread>
#include <iioutil/connection.h>

namespace scopy::swiot {
class SwiotIdentifyTask : public QThread
{
public:
	SwiotIdentifyTask(QString uri, QObject *parent = nullptr);
	~SwiotIdentifyTask();
	void run() override;

private:
	QString m_uri;
	Connection *m_conn;
};
} // namespace scopy::swiot
#endif // SWIOTIDENTIFYTASK_H
