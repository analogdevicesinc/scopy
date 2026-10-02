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

#include "filereader.h"

#include "csvfilereader.h"
#include "wavfilereader.h"

#include <QFileInfo>

namespace scopy {
namespace adc {

const QVector<float> &FileReader::emptyChannel()
{
	static const QVector<float> empty;
	return empty;
}

std::unique_ptr<FileReader> makeFileReader(const QString &path)
{
	const QString suffix = QFileInfo(path).suffix().toLower();

	if(suffix == QStringLiteral("csv") || suffix == QStringLiteral("txt")) {
		return std::make_unique<CsvFileReader>();
	}
	if(suffix == QStringLiteral("wav")) {
		return std::make_unique<WavFileReader>();
	}

	return nullptr;
}

QString fileReaderFilter()
{
	QStringList patterns{QStringLiteral("*.csv"), QStringLiteral("*.txt"), QStringLiteral("*.wav")};
	QStringList groups{QStringLiteral("CSV / text (*.csv *.txt)"), QStringLiteral("WAV audio (*.wav)")};

	groups.prepend(QStringLiteral("Supported files (%1)").arg(patterns.join(QChar(' '))));
	groups.append(QStringLiteral("All files (*)"));
	return groups.join(QStringLiteral(";;"));
}

} // namespace adc
} // namespace scopy
