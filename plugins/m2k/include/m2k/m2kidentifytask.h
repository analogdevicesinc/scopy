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

#ifndef M2KIDENTIFYTASK_H
#define M2KIDENTIFYTASK_H

#include <QThread>
namespace scopy::m2k {
class M2kIdentifyTask : public QThread
{
public:
	M2kIdentifyTask(QString uri, QObject *parent = nullptr);
	~M2kIdentifyTask();
	void run() override;

private:
	QString m_uri;
};
} // namespace scopy::m2k
#endif // M2KIDENTIFYTASK_H
