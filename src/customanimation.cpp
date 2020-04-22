/*
 * Copyright 2018 Analog Devices, Inc.
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

#include "customanimation.h"

#include <animationmanager.h>

using namespace adiscope;

CustomAnimation::CustomAnimation(QObject* target)
	: QPropertyAnimation(target)
	, m_enabled(true)
	, m_duration(0)
{
	AnimationManager::getInstance().registerAnimation(this);
}

CustomAnimation::CustomAnimation(QObject* target, const QByteArray& propertyName, QObject* parent)
	: QPropertyAnimation(target, propertyName, parent)
	, m_enabled(true)
	, m_duration(0)
{
	AnimationManager::getInstance().registerAnimation(this);
}

CustomAnimation::~CustomAnimation() {}

void CustomAnimation::setDuration(int msec)
{
	m_duration = msec;
	if (m_enabled) {
		QPropertyAnimation::setDuration(msec);
	}
}

void CustomAnimation::toggle(bool enabled)
{
	m_enabled = enabled;
	QPropertyAnimation::setDuration(enabled ? m_duration : 0);
}
