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

#include "animationmanager.h"

using namespace adiscope;
AnimationManager &AnimationManager::getInstance() {
	static AnimationManager INSTANCE;
	return INSTANCE;
}

void AnimationManager::toggleAnimations(bool on) {
	m_animationsEnabled = on;
	Q_EMIT toggle(on);
}

void AnimationManager::registerAnimation(CustomAnimation *animation) {
	// register the animation for enable/disable signal
	connect(this, &AnimationManager::toggle, animation,
		&CustomAnimation::toggle);
	animation->toggle(m_animationsEnabled);
}

AnimationManager::AnimationManager() : m_animationsEnabled(true) {}
