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

#include <animation_manager.hpp>
#include <scopy/gui/custom_animation.hpp>

using namespace scopy::gui;

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
