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

#include "attrinstrumenthandler.h"
#include <pluginbase/preferences.h>

using namespace scopy::pqm;

AttrInstrumentHandler::AttrInstrumentHandler(QString uri, QObject *parent)
	: QObject(parent)
	, m_uri(uri)
{
	m_concurrentAcq = Preferences::get("pqm_concurrent").toBool();
	connect(Preferences::GetInstance(), &Preferences::preferenceChanged, this,
		&AttrInstrumentHandler::concurrentEnable);
}

AttrInstrumentHandler::~AttrInstrumentHandler() { ResourceManager::close("pqm" + m_uri); }

void AttrInstrumentHandler::stop()
{
	if(m_harmonicsInstrument) {
		m_harmonicsInstrument->stop();
	}
	if(m_rmsInstrument) {
		m_rmsInstrument->stop();
	}
	resetValues();
}

void AttrInstrumentHandler::onToggle(bool en, const QString &tool)
{
	m_runningMap[tool] = en;
	if(!m_concurrentAcq) {
		resourceManagerCheck(en);
	}
}

void AttrInstrumentHandler::concurrentEnable(QString pref, QVariant value)
{
	if(pref != "pqm_concurrent") {
		return;
	}
	m_concurrentAcq = value.toBool();
	if(!m_concurrentAcq && m_rmsInstrument && m_harmonicsInstrument) {
		m_rmsInstrument->stop();
		m_harmonicsInstrument->stop();
	}
}

bool AttrInstrumentHandler::isAnyToolRunning() { return m_runningMap[HARMONICS_TOOL] || m_runningMap[RMS_TOOL]; }

void AttrInstrumentHandler::resourceManagerCheck(bool en)
{
	if(en && !m_resourceLock) {
		ResourceManager::open("pqm" + m_uri, this);
		m_resourceLock = true;
	} else if(!en && m_resourceLock) {
		if(isAnyToolRunning()) {
			return;
		}
		ResourceManager::close("pqm" + m_uri);
		m_resourceLock = false;
	}
}

void AttrInstrumentHandler::resetValues()
{
	m_resourceLock = false;
	m_runningMap[HARMONICS_TOOL] = false;
	m_runningMap[RMS_TOOL] = false;
}

void AttrInstrumentHandler::setRmsInstrument(RmsInstrument *newRmsInstrument)
{
	if(m_rmsInstrument) {
		return;
	}
	m_rmsInstrument = newRmsInstrument;
	connect(m_rmsInstrument, &RmsInstrument::enableTool, this, &AttrInstrumentHandler::onToggle);
}

void AttrInstrumentHandler::setHarmonicsInstrument(HarmonicsInstrument *newHarmonicsInstrument)
{
	if(m_harmonicsInstrument) {
		return;
	}
	m_harmonicsInstrument = newHarmonicsInstrument;
	connect(m_harmonicsInstrument, &HarmonicsInstrument::enableTool, this, &AttrInstrumentHandler::onToggle);
}
