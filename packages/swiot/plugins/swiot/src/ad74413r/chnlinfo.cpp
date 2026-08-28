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

#include "ad74413r/chnlinfo.h"

#include <component/attribute.h>
#include <math.h>

using namespace scopy::swiot;
using namespace scopy;

ChnlInfo::ChnlInfo(QString plotUm, QString hwUm, component::Channel *chnl)
	: m_isOutput(false)
	, m_isEnabled(false)
	, m_isScanElement(false)
	, m_scanIndex(-1)
	, m_plotUm(plotUm)
	, m_hwUm(hwUm)
	, m_chnl(chnl)
{
	m_offsetScalePair = {0, 1};
	if(m_chnl) {
		m_chnlId = m_chnl->id();
		m_isOutput = m_chnl->isOutput();
		readScaleOffset();
		initUnitOfMeasureFactor();
	}
}

ChnlInfo::~ChnlInfo() {}

void ChnlInfo::readScaleOffset()
{
	if(!m_chnl) {
		return;
	}
	component::Attribute *scaleAttr =
		m_chnl->findChild<component::Attribute *>("scale", Qt::FindDirectChildrenOnly);
	if(scaleAttr && scaleAttr->readCapability()) {
		QCoro::waitFor(scaleAttr->readCapability()->readAsync());
		bool ok = false;
		double scale = scaleAttr->cachedValue().toDouble(&ok);
		if(ok) {
			m_offsetScalePair.second = scale;
		}
	}
	component::Attribute *offsetAttr =
		m_chnl->findChild<component::Attribute *>("offset", Qt::FindDirectChildrenOnly);
	if(offsetAttr && offsetAttr->readCapability()) {
		QCoro::waitFor(offsetAttr->readCapability()->readAsync());
		bool ok = false;
		double offset = offsetAttr->cachedValue().toDouble(&ok);
		if(ok) {
			m_offsetScalePair.first = offset;
		}
	}
}

component::Channel *ChnlInfo::chnl() const { return m_chnl; }

bool ChnlInfo::isOutput() const { return m_isOutput; }

bool ChnlInfo::isScanElement() const { return m_isScanElement; }

long ChnlInfo::scanIndex() const { return m_scanIndex; }

void ChnlInfo::setScanIndex(long index)
{
	m_scanIndex = index;
	m_isScanElement = (index >= 0);
}

QString ChnlInfo::chnlId() const { return m_chnlId; }

std::pair<int, int> ChnlInfo::rangeValues() const { return m_rangeValues; }

std::pair<double, double> ChnlInfo::offsetScalePair() const { return m_offsetScalePair; }

bool ChnlInfo::isEnabled() const { return m_isEnabled; }

void ChnlInfo::setIsEnabled(bool newIsEnabled) { m_isEnabled = newIsEnabled; }

QString ChnlInfo::unitOfMeasure() const { return m_plotUm; }

void ChnlInfo::initUnitOfMeasureFactor()
{
	m_unitOfMeasureFactor["G"] = pow(10, 9);
	m_unitOfMeasureFactor["M"] = pow(10, 6);
	m_unitOfMeasureFactor["K"] = pow(10, 3);
	m_unitOfMeasureFactor["m"] = pow(10, -3);
	m_unitOfMeasureFactor["u"] = pow(10, -6);
	m_unitOfMeasureFactor["µ"] = pow(10, -6);
	m_unitOfMeasureFactor["n"] = pow(10, -9);
	m_unitOfMeasureFactor["p"] = pow(10, -12);
}

#include "moc_chnlinfo.cpp"
