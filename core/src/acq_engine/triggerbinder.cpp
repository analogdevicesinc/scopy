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

#include "triggerbinder.h"

namespace scopy {
namespace acq {

TriggerBinder::TriggerBinder(TriggerProcessor *trig, AcquisitionEngine *engine, QObject *parent)
	: QObject(parent)
	, m_trig(trig)
	, m_engine(engine)
{}

TriggerBinder::~TriggerBinder()
{
	if(m_singleShotConn)
		QObject::disconnect(m_singleShotConn);
	if(m_replotConn)
		QObject::disconnect(m_replotConn);
}

void TriggerBinder::armSingleShot()
{
	if(!m_trig || !m_engine)
		return;
	if(m_singleShotConn)
		QObject::disconnect(m_singleShotConn); // re-arm replaces the prior wire

	m_singleShotArmed = true;
	m_singleShotConn = connect(
		m_trig, &TriggerProcessor::fired, this,
		[this](quint32 sampleIndex, QMap<QString, scopy::acq::SampleVariant> snapshot) {
			// The worker keeps running until stop() lands, so a second
			// fire may already be queued behind this one. Drop it.
			if(!m_singleShotArmed)
				return;
			m_singleShotArmed = false;
			if(m_singleShotConn) {
				QObject::disconnect(m_singleShotConn);
				m_singleShotConn = {};
			}
			Q_EMIT singleShotFired(sampleIndex, std::move(snapshot));
			if(m_engine)
				m_engine->stop();
		},
		Qt::QueuedConnection);
}

void TriggerBinder::disarmSingleShot()
{
	m_singleShotArmed = false;
	if(m_singleShotConn) {
		QObject::disconnect(m_singleShotConn);
		m_singleShotConn = {};
	}
}

void TriggerBinder::bindReplotOnFire()
{
	if(!m_trig)
		return;
	if(m_replotConn)
		QObject::disconnect(m_replotConn);
	m_replotConn =
		connect(m_trig, &TriggerProcessor::fired, this, &TriggerBinder::replotFired, Qt::QueuedConnection);
}

void TriggerBinder::unbindReplotOnFire()
{
	if(m_replotConn) {
		QObject::disconnect(m_replotConn);
		m_replotConn = {};
	}
}

} // namespace acq
} // namespace scopy

#include "moc_triggerbinder.cpp"
