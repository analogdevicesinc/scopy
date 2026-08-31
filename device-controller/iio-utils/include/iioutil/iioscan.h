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

#pragma once

#include "iioutil/handles.h"

#include <QObject>
#include <QVector>

namespace scopy::iio {

class IScanOps;
class IBackend;

class IIOScan : public QObject
{
	Q_OBJECT
public:
	explicit IIOScan(LibiioVersion version = LibiioVersion::V0, QObject *parent = nullptr);
	~IIOScan();

	QVector<ScanResult> scan(const QString &backends = QString());
	void scanAsync(const QString &backends = QString());

Q_SIGNALS:
	void scanCompleted(const QVector<scopy::iio::ScanResult> &results);

private:
	IBackend *m_backend = nullptr; // non-owning; loader keeps backends resident for the process
	IScanOps *m_scanOps = nullptr;
};

} // namespace scopy::iio
