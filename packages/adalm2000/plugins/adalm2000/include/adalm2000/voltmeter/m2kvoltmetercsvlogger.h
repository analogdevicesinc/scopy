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

#pragma once

#include "scopy-adalm2000_export.h"

#include <QFile>
#include <QObject>
#include <QTextStream>

namespace scopy::adalm2000 {

class SCOPY_ADALM2000_EXPORT M2kVoltmeterCsvLogger : public QObject
{
	Q_OBJECT
public:
	explicit M2kVoltmeterCsvLogger(QObject *parent = nullptr);
	~M2kVoltmeterCsvLogger() override;

	bool open(const QString &path);
	void append(double ch0, double ch1);
	void close();
	bool isOpen() const;

private:
	QFile m_file;
	QTextStream m_stream;
};

} // namespace scopy::adalm2000
