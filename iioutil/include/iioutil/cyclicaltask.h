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

#ifndef CYCLICALTASK_H
#define CYCLICALTASK_H

#include "scopy-iioutil_export.h"

#include <iio.h>

#include <QLoggingCategory>
#include <QObject>
#include <QThread>
#include <QTimer>

namespace scopy {
/**
 * @brief The CyclicalTask class
 * Implements a way to cyclically call QThreads
 */
class SCOPY_IIOUTIL_EXPORT CyclicalTask : public QObject
{
	Q_OBJECT
public:
	CyclicalTask(QThread *task, QObject *parent = nullptr);
	~CyclicalTask();
	void start(int period = 5000);
	void stop();

private Q_SLOTS:
	void startThread();

private:
	QTimer *t;
	bool enabled = false;
	QThread *task;
	const int THREAD_FINISH_TIMEOUT = 30000;
};
} // namespace scopy

#endif // CYCLICALTASK_H
