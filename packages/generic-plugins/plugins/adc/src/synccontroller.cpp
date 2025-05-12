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

#include "synccontroller.h"

namespace scopy {
namespace adc {

SyncController::SyncController(QObject *parent) {}

SyncController::~SyncController() {}

void SyncController::addInstrument(SyncInstrument *s)
{
	m_syncInstruments.append(s);
	s->setSyncController(this);
	m_syncState.insert(s, false);
}

void SyncController::removeInstrument(SyncInstrument *s)
{
	m_syncInstruments.removeAll(s);
	s->setSyncController(nullptr);
	m_syncState.remove(s);
}

void SyncController::arm(SyncInstrument *si)
{
	if(!si->syncMode()) {
		si->onArm();
	} else {
		for(SyncInstrument *s : qAsConst(m_syncInstruments)) {
			if(s->syncMode()) {
				s->onArm();
			}
		}
	}
}

void SyncController::disarm(SyncInstrument *si)
{
	if(!si->syncMode()) {
		si->onDisarm();
	} else {
		for(SyncInstrument *s : qAsConst(m_syncInstruments)) {
			if(s->syncMode()) {
				s->onDisarm();
			}
		}
	}
}

/*void SyncController::sync(SyncInstrument *s) {
	m_syncState[s] = true;
	bool reset = true;
	for(auto sync : m_syncState.keys()) {
		if(m_syncState[sync] == false) {
			reset = false;
		}
	}
	if(reset == true) {
		for(auto sync : m_syncState.keys()) {
			m_syncState[sync] = false;
		}
		Q_EMIT resetAll();
	}
}*/

void SyncController::setBufferSize(SyncInstrument *si, uint32_t newBufferSize)
{
	if(!si->syncMode())
		return;
	for(SyncInstrument *s : qAsConst(m_syncInstruments)) {
		if(s == si)
			continue;
		if(s->syncMode()) {
			s->setSyncBufferSize(newBufferSize);
		}
	}
}

void SyncController::setSingleShot(SyncInstrument *si, bool newSingleShot)
{
	if(!si->syncMode())
		return;
	for(SyncInstrument *s : qAsConst(m_syncInstruments)) {
		if(s == si)
			continue;
		if(s->syncMode()) {
			s->setSyncSingleShot(newSingleShot);
		}
	}
}
} // namespace adc
} // namespace scopy
