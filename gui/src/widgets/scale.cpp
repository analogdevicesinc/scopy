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

#include "scale.h"

namespace scopy {
namespace gui {

Scale::Scale(QString unit, double min, double max, bool hasPrefix)
	: m_unit(unit)
	, m_min(min)
	, m_max(max)
	, m_hasPrefix(hasPrefix)
{
	m_scaleCb = new QComboBox();

	if(hasPrefix) {
		// PRESET METRIC PREFIX
		m_scalePrefixes.append({QString("n"), 1e-9});
		m_scalePrefixes.append({QString("u"), 1e-6});
		m_scalePrefixes.append({QString("m"), 1e-3});
		m_scalePrefixes.append({QString(""), 1e0});
		m_scalePrefixes.append({QString("k"), 1e3});
		m_scalePrefixes.append({QString("M"), 1e6});
		m_scalePrefixes.append({QString("G"), 1e9});
	}

	m_scaleOptions.append({m_unit, 1e0});
	populateScaleCb();
	m_scaleCb->setCurrentIndex(0);
}

Scale::~Scale()
{
	m_scaleOptions.clear();
	m_scalePrefixes.clear();
}

double Scale::getScaleForPrefix(QString prefix, Qt::CaseSensitivity s)
{
	for(int i = 0; i < m_scalePrefixes.count(); i++) {

		if(s == Qt::CaseSensitive) {
			if(m_scalePrefixes[i].prefix == prefix) {
				return m_scalePrefixes[i].scale;
			}
		} else {
			if(m_scalePrefixes[i].prefix.toLower() == prefix.toLower()) {
				return m_scalePrefixes[i].scale;
			}
		}
	}
	return -1;
}

double Scale::getScaleForUnit(QString unit, Qt::CaseSensitivity s)
{
	for(int i = 0; i < m_scaleOptions.count(); i++) {
		if(s == Qt::CaseSensitive) {
			if(m_scaleOptions[i].option == unit) {
				return m_scaleOptions[i].scale;
			}
		} else {
			if(m_scaleOptions[i].option.toLower() == unit.toLower()) {
				return m_scaleOptions[i].scale;
			}
		}
	}
	return -1;
}

double Scale::getScaleForSymbol(QString symbol)
{
	if(m_hasPrefix) {
		// TODO apply some filters for unit of measure ??
		double value = getScaleForPrefix(symbol.mid(0, 1), Qt::CaseSensitive);
		if(value == -1) {
			value = getScaleForPrefix(symbol.mid(0, 1), Qt::CaseInsensitive);
		}
		return value;
	} else {
		double value = getScaleForUnit(symbol, Qt::CaseSensitive);
		if(value == -1) {
			value = getScaleForUnit(symbol, Qt::CaseInsensitive);
		}
		return value;
	}
}

bool Scale::scalingEnabled() const { return m_scalingEnabled; }

void Scale::setScalingEnabled(bool newScalingEnabled)
{
	m_scalingEnabled = newScalingEnabled;
	m_scaleCb->setVisible(newScalingEnabled);
}

QComboBox *Scale::scaleCb() const { return m_scaleCb; }

QString Scale::unit() const { return m_unit; }

void Scale::setUnit(const QString &newUnit)
{
	m_unit = newUnit;
	// when using custom scales changing the unit also resets the scale
	if(!m_hasPrefix) {
		// reset scale options to new unit
		m_scaleOptions.clear();
		m_scaleOptions.append({m_unit, 1e0});
	}
	populateScaleCb();
}

void Scale::computeScale(double value)
{
	// if there is only one option in the scale use that
	if(m_scaleCb->count() == 0) {
		return;
	}
	// use absolute value when computing scale
	double val = abs(value);
	// check if value can be upscaled or downscaled if scaled is changed update scale
	int curretnScaleIndex = m_scaleCb->currentIndex();
	// value is bigger than current scale
	if(val > m_scaleCb->itemData(curretnScaleIndex).toDouble()) {
		if(curretnScaleIndex != m_scaleCb->count() - 1) {
			int i = 0;
			// find coresponding upper scale
			while((val >= m_scaleCb->itemData(curretnScaleIndex + i).toDouble()) &&
			      ((curretnScaleIndex + i) <= m_scaleCb->count() - 1)) {
				i++;
			}
			m_scaleCb->setCurrentIndex(curretnScaleIndex + i - 1);
		}
	} else {
		// value is lower than current scale
		int i = curretnScaleIndex;
		// find coresponding lower scale
		while((val < m_scaleCb->itemData(i).toDouble()) && (i > 0)) {
			i--;
		}
		m_scaleCb->setCurrentIndex(i);
	}
}

bool Scale::hasPrefix() const { return m_hasPrefix; }

void Scale::setHasPrefix(bool newHasPrefix)
{
	m_hasPrefix = newHasPrefix;
	populateScaleCb();
}

void Scale::populateScaleCb()
{
	m_scaleCb->clear();
	if(m_hasPrefix) {
		for(int i = 0; i < m_scalePrefixes.count(); i++) {
			auto scale = m_scalePrefixes[i].scale;
			m_scaleCb->addItem(m_scalePrefixes[i].prefix + m_unit, scale);
		}
	} else {
		for(int i = 0; i < m_scaleOptions.count(); i++) {
			m_scaleCb->addItem(m_scaleOptions[i].option, m_scaleOptions[i].scale);
		}
	}

	Q_EMIT scaleUpdated();
}

QList<UnitPrefix> Scale::scalePrefixes() const { return m_scalePrefixes; }

void Scale::setScalePrefixes(const QList<UnitPrefix> &newScalePrefixes)
{
	m_scalePrefixes = newScalePrefixes;
	populateScaleCb();
}

QList<ScaleOption> Scale::scaleOptions() const { return m_scaleOptions; }

void Scale::setScaleOptions(const QList<ScaleOption> &newScaleOptions)
{
	m_scaleOptions = newScaleOptions;
	populateScaleCb();
}

} // namespace gui
} // namespace scopy

#include "moc_scale.cpp"
