/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef DATAWRITER_H
#define DATAWRITER_H

#include "common/scopyconfig.h"
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <qdir.h>

namespace scopy::extprocplugin {
class DataWriter : public QObject
{
	Q_OBJECT
public:
	DataWriter(QObject *parent = nullptr);
	~DataWriter();

	uchar *mappedData();
	bool openFile(const QString &path, int64_t dataSize);
	void unmap();
	QFileInfo getFileInfo();

private:
	QFile m_file;
	uchar *m_data = nullptr;
	int64_t m_dataSize = 0;
};
} // namespace scopy::extprocplugin

#endif // DATAWRITER_H
