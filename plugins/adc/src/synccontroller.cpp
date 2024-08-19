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
