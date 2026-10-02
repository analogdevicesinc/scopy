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

#ifndef FILEREADER_H
#define FILEREADER_H

#include <memory>
#include <QString>
#include <QStringList>
#include <QVector>

namespace scopy {
namespace adc {

class FileReader
{
public:
	virtual ~FileReader() = default;

	virtual bool open(const QString &path, QString *err) = 0;

	virtual QStringList channels() const = 0;
	// Every channel has the same length, so samples(0).size() is the file's length.
	virtual const QVector<float> &samples(int index) const = 0;

	virtual double sampleRate() const { return 0.0; }

	virtual QString unit(int index) const
	{
		Q_UNUSED(index)
		return {};
	}

protected:
	static const QVector<float> &emptyChannel();
};

std::unique_ptr<FileReader> makeFileReader(const QString &path);
QString fileReaderFilter();

} // namespace adc
} // namespace scopy

#endif // FILEREADER_H
