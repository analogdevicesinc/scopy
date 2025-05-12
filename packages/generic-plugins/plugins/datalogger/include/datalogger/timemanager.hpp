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

#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <QDateTime>
#include <QObject>
#include <QTimer>

namespace scopy {
namespace datamonitor {

class TimeManager : public QObject
{
	Q_OBJECT

protected:
	TimeManager(QObject *parent = nullptr);
	~TimeManager();

public:
	TimeManager(TimeManager &other) = delete;
	static TimeManager *GetInstance();

	void setStartTime();
	double startTime() const;
	QDateTime startTime();

	void startTimer();
	void stopTimer();
	void setTimerInterval(double interval);

	bool isRunning() const;
	void setIsRunning(bool newIsRunning);

	QDateTime lastReadValue() const;

Q_SIGNALS:
	void timeout();
	void toggleRunning(bool toggled);

private:
	static TimeManager *pinstance_;

	QDateTime m_startTime;
	bool m_realTime = false;
	QTimer *m_timer;
	bool m_isRunning = false;

	QDateTime m_lastReadValue;
};
} // namespace datamonitor
} // namespace scopy
#endif // TIMEMANAGER_H
