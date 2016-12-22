/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "state_updater.h"

using namespace adiscope;

StateUpdater::StateUpdater(int msecTimeout, QObject *parent):
	QObject(parent),
	m_enabled(false),
	m_off_state(0),
	m_idle_state(0),
	m_last_input(0),
	m_output_state(0),
	m_timeout(msecTimeout)

{
	m_timer.setSingleShot(true);
	m_timer.setTimerType(Qt::CoarseTimer);

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
}

void StateUpdater::onTimerTimeout()
{
	m_output_state = m_idle_state;
	Q_EMIT outputChanged(m_output_state);
}

bool StateUpdater::enabled() const
{
	return m_enabled;
}

void StateUpdater::setEnabled(bool en)
{
	if (m_enabled != en) {
		m_enabled = en;

		if (en) {
			m_output_state = m_idle_state;
		} else {
			if (m_timer.isActive())
				m_timer.stop();
			m_output_state = m_off_state;
		}
		Q_EMIT outputChanged(m_output_state);
	}
}

int StateUpdater::timeout() const
{
	return m_timeout;
}

void StateUpdater::setTimeout(int msec)
{
	m_timeout = msec;
}

int StateUpdater::idleState() const
{
	return m_idle_state;
}

void StateUpdater::setIdleState(int state)
{
	m_idle_state = state;
}

int StateUpdater::offState() const
{
	return m_off_state;
}

void StateUpdater::setOffState(int state)
{
	m_off_state = state;
}

int StateUpdater::inputState() const
{
	return m_last_input;
}

void StateUpdater::setInput(int input)
{
	if (!m_enabled)
		return;

	int old_output_state = m_output_state;

	m_last_input = input;
	m_output_state = input;

	m_timer.start(m_timeout);

	if (old_output_state != input)
		Q_EMIT outputChanged(input);
}

int StateUpdater::outputState() const
{
	return m_output_state;
}
