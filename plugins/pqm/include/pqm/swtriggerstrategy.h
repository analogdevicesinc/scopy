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

#ifndef SWTRIGGERSTRATEGY_H
#define SWTRIGGERSTRATEGY_H

#include <plottingstrategy.h>

namespace scopy::pqm {

class SwTriggerStrategy : public PlottingStrategy
{
public:
	SwTriggerStrategy(int samplingFreq, QString triggeredBy);
	~SwTriggerStrategy();

	QMap<QString, QVector<double>> processSamples(QMap<QString, QVector<double>> samples) override;

private:
	void zeroCrossing(QMap<QString, QVector<double>> samples);
	bool m_fill;
	QString m_triggeredBy;
};

} // namespace scopy::pqm

#endif // SWTRIGGERSTRATEGY_H
