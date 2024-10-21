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

#ifndef SCANBUTTONCONTROLLER_H
#define SCANBUTTONCONTROLLER_H

#include "iioutil/cyclicaltask.h"
#include "scopy-core_export.h"

#include <QObject>
#include <QPushButton>

namespace scopy {
class SCOPY_CORE_EXPORT ScanButtonController : public QObject
{
	Q_OBJECT
public:
	explicit ScanButtonController(CyclicalTask *cs, QPushButton *btn, QObject *parent);
	~ScanButtonController();

	int scanTimeout() const;
	void setScanTimeout(int newScanTimeout);

public Q_SLOTS:
	void enableScan(bool b);
	void startScan();
	void stopScan();

private:
	QPushButton *btn;
	CyclicalTask *cs;
	QMetaObject::Connection conn;

	int m_scanTimeout;
};
} // namespace scopy

#endif // SCANBUTTONCONTROLLER_H
