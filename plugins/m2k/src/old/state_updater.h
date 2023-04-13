/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STATE_UPDATER_H
#define STATE_UPDATER_H

#include <QObject>
#include <QTimer>

namespace scopy::m2k {

class StateUpdater: public QObject
{
	Q_OBJECT

public:
	StateUpdater(int msecTimeout, QObject *parent = 0);

	bool enabled() const;
	int timeout() const;
	int idleState() const;
	int offState() const;
	int inputState() const;
	int outputState() const;

public Q_SLOTS:
	void setEnabled(bool en);
	void setTimeout(int msec);
	void setIdleState(int state);
	void setOffState(int state);
	void setInput(int input);

Q_SIGNALS:
	void outputChanged(int);

private Q_SLOTS:
	void onTimerTimeout();

private:
	bool m_enabled;
	int m_off_state;
	int m_idle_state;
	int m_last_input;
	int m_output_state;

	int m_timeout;
	QTimer m_timer;
};

} // namespace scopy

#endif // STATE_UPDATER_H
