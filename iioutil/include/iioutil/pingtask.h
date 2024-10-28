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

#ifndef PINGTASK_H
#define PINGTASK_H

#include "scopy-iioutil_export.h"
#include <QThread>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT PingTask : public QThread
{
	Q_OBJECT
public:
	PingTask(QObject *parent);
	~PingTask();

	virtual void run() override;
	virtual bool ping() = 0;

Q_SIGNALS:
	void pingSuccess();
	void pingFailed();
};
} // namespace scopy

#endif // PINGTASK_H
