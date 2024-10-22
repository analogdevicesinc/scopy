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

#ifndef SCANNEDIIOCONTEXTCOLLECTOR_H
#define SCANNEDIIOCONTEXTCOLLECTOR_H

#include "device.h"
#include "scopy-core_export.h"

#include <QObject>
#include <QSet>

namespace scopy {
class SCOPY_CORE_EXPORT ScannedIIOContextCollector : public QObject
{
	Q_OBJECT
public:
	explicit ScannedIIOContextCollector(QObject *parent = nullptr);
	~ScannedIIOContextCollector();

public Q_SLOTS:
	void update(QVector<QPair<QString, QString>> ctxsDescription);
	void clearCache();
	void lock(QString, Device *);
	void unlock(QString, Device *);
	void removeDevice(QString id, Device *d);
Q_SIGNALS:
	void foundDevice(QString cat, QString uri);
	void lostDevice(QString cat, QString uri);

private:
	QSet<QString> uris;
	QSet<QString> lockedUris;
};
} // namespace scopy

#endif // SCANNEDIIOCONTEXTCOLLECTOR_H
