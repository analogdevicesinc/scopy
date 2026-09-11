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
 *
 */

#include "iioutil/iioscan.h"
#include "iioutil/iscanops.h"
#include <QThreadPool>

namespace scopy::iio {

IIOScan::IIOScan(IScanOps *scanOps, QObject *parent)
	: QObject(parent)
	, m_scanOps(scanOps)
{}

IIOScan::~IIOScan() {}

QVector<ScanResult> IIOScan::scan(const QString &backends) { return m_scanOps->scan(backends); }

void IIOScan::scanAsync(const QString &backends)
{
	QThreadPool::globalInstance()->start([this, backends]() {
		QVector<ScanResult> results = m_scanOps->scan(backends);
		Q_EMIT scanCompleted(results);
	});
}

} // namespace scopy::iio
