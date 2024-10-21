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

#ifndef IIOSCANTASK_H
#define IIOSCANTASK_H

#include "scopy-iioutil_export.h"

#include <QThread>

namespace scopy {
/**
 * @brief The IIOScanTask class
 * IIOScanTask - scans for IIO context and emits a scanFinished signal
 */
class SCOPY_IIOUTIL_EXPORT IIOScanTask : public QThread
{
	Q_OBJECT
public:
	IIOScanTask(QObject *parent);
	~IIOScanTask();

	virtual void run() override;
	void setScanParams(QString s);
	static int scan(QVector<QPair<QString, QString>> *ctxs, QString scanParams);
	static QVector<QString> getSerialPortsName();

Q_SIGNALS:
	void scanFinished(QVector<QPair<QString, QString>> ctxs);

protected:
	static QString parseDescription(const QString &d);
	QString scanParams = "";
	bool enabled;
};
} // namespace scopy
#endif // IIOSCANTASK_H
