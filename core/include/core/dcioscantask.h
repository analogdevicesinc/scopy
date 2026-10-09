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

#ifndef DCIIOSCANTASK_H
#define DCIIOSCANTASK_H

#include "scopy-core_export.h"

#include <QThread>

namespace scopy {
// Scans for IIO contexts through the device-controller backend (libiio v0 or v1,
// fixed at build time) and emits scanFinished. Drop-in replacement for the former
// IIOScanTask: same QThread contract, so it plugs into CyclicalTask / ScanButtonController
// / ScannedIIOContextCollector unchanged. Lives in core (not iioutil) because it uses the
// component layer, which core links and iioutil must not.
class SCOPY_CORE_EXPORT DcIioScanTask : public QThread
{
	Q_OBJECT
public:
	DcIioScanTask(QObject *parent);
	~DcIioScanTask();

	void run() override;
	void setScanParams(QString s);
	// Appends {description, uri} for each found context. Returns the number found, or -1 on error.
	static int scan(QVector<QPair<QString, QString>> *ctxs, QString scanParams);

Q_SIGNALS:
	void scanFinished(QVector<QPair<QString, QString>> ctxs);

protected:
	static QString parseDescription(const QString &d);
	QString scanParams = "";
};
} // namespace scopy
#endif // DCIIOSCANTASK_H
