/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 */

#include "rangepresetscaler.h"

using namespace scopy;

RangePresetScaler::RangePresetScaler()
	: m_presets({{-0.1, 0.1, 5, 5}, {-1.0, 1.0, 5, 5}, {-5.0, 5.0, 10, 2}, {-25.0, 25.0, 10, 5}})
{
	resetWindow();
}

void RangePresetScaler::setPresets(const QVector<ScalePreset> &presets)
{
	if(presets.isEmpty()) {
		return;
	}
	m_presets = presets;
	m_index = 0;
	resetWindow();
}

QVector<ScalePreset> RangePresetScaler::presets() const { return m_presets; }

void RangePresetScaler::setFloorAtZero(bool floorAtZero)
{
	m_floorAtZero = floorAtZero;
	m_index = 0;
	resetWindow();
}

bool RangePresetScaler::floorAtZero() const { return m_floorAtZero; }

int RangePresetScaler::index() const { return m_index; }

ScalePreset RangePresetScaler::current() const { return presetAt(m_index); }

ScalePreset RangePresetScaler::presetAt(int i) const
{
	ScalePreset p = m_presets.at(i);
	if(m_floorAtZero && p.lower < 0.0) {
		p.lower = 0.0;
	}
	return p;
}

void RangePresetScaler::resetWindow()
{
	const ScalePreset smallest = presetAt(0);
	m_min = smallest.lower;
	m_max = smallest.upper;
}

int RangePresetScaler::select(int i)
{
	m_index = i;
	return i;
}

int RangePresetScaler::push(double value)
{
	if(value < m_min) {
		m_min = value;
	} else if(value > m_max) {
		m_max = value;
	}

	const ScalePreset cur = presetAt(m_index);
	if(value < cur.lower) {
		for(int i = 0; i < m_presets.size(); ++i) {
			if(presetAt(i).lower <= value) {
				return select(i);
			}
		}
	} else if(value > cur.upper) {
		for(int i = 0; i < m_presets.size(); ++i) {
			if(presetAt(i).upper >= value) {
				return select(i);
			}
		}
	}
	return -1;
}

int RangePresetScaler::settle()
{
	int next = -1;
	for(int i = 0; i < m_presets.size(); ++i) {
		const ScalePreset p = presetAt(i);
		if(p.lower <= m_min && p.upper >= m_max) {
			next = (i == m_index) ? -1 : select(i);
			break;
		}
	}
	resetWindow();
	return next;
}
