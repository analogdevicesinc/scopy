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


#ifndef SWIOTPINGTASK_H
#define SWIOTPINGTASK_H

#include <QThread>
#include <iio.h>
#include "scopy-swiot_export.h"
#include <iioutil/command.h>

namespace scopy::swiot {
class SwiotPingTask : public QThread {
	Q_OBJECT
public:
	SwiotPingTask(iio_context *c, QObject *parent = nullptr);
	~SwiotPingTask();
	virtual void run() override;
Q_SIGNALS:
	void pingSuccess();
	void pingFailed();
protected:
	iio_context *c;
	bool enabled;

private Q_SLOTS:
	void getTriggerCommandFinished(scopy::Command *cmd);
};
}
#endif // SWIOTPINGTASK_H
