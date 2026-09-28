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

#ifndef WAVFILEREADER_H
#define WAVFILEREADER_H

#include "filereader.h"

namespace scopy {
namespace adc {

class WavFileReader : public FileReader
{
public:
	bool open(const QString &path, QString *err) override;

	QStringList channels() const override { return m_names; }
	const QVector<float> &samples(int index) const override;
	double sampleRate() const override { return m_sampleRate; }
	QString unit(int index) const override;

	int bitsPerSample() const { return m_bits; }

private:
	QList<QVector<float>> m_data;
	QStringList m_names;
	double m_sampleRate{0.0};
	int m_bits{0};
};

} // namespace adc
} // namespace scopy

#endif // WAVFILEREADER_H
