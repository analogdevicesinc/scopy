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

#ifndef CSVFILEREADER_H
#define CSVFILEREADER_H

#include "filereader.h"

namespace scopy {
namespace adc {

class CsvFileReader : public FileReader
{
public:
	enum class FirstColumnMode
	{
		Auto,
		Time,
		Data
	};

	bool open(const QString &path, QString *err) override;

	QStringList channels() const override;
	const QVector<float> &samples(int index) const override;
	double sampleRate() const override { return m_effectiveRate; }

	void setFirstColumnMode(FirstColumnMode mode);
	FirstColumnMode firstColumnMode() const { return m_mode; }
	bool usesFirstColumnAsTime() const { return m_timeIsFirst; }
	int malformedCells() const { return m_malformed; }

private:
	void resolveFirstColumn();

	QList<QVector<float>> m_columns;
	QStringList m_names;

	FirstColumnMode m_mode{FirstColumnMode::Auto};
	bool m_timeIsFirst{false};
	bool m_scopyHeader{false};
	int m_malformed{0};
	double m_declaredRate{0.0};
	double m_effectiveRate{0.0};
};

} // namespace adc
} // namespace scopy

#endif // CSVFILEREADER_H
