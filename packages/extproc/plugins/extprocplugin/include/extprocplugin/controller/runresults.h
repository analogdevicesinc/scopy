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

#ifndef RUNRESULTS_H
#define RUNRESULTS_H

#include <QString>
#include <QVariant>
#include <QVariantMap>

namespace scopy::extprocplugin {
class RunResults
{
public:
	RunResults();
	~RunResults() = default;

	bool isValid();
	QVariantMap getResultForAnalysis(QString type);
	QVariant getMeasurement(QString name);

	QString getOutputFile() const;
	void setOutputFile(const QString &outputFile);

	QVariantMap getResults() const;
	void setResults(const QVariantMap &results);

	QVariantMap getMeasurements() const;
	void setMeasurements(const QVariantMap &measurements);

private:
	QString m_outputFile;
	QVariantMap m_results;
	QVariantMap m_measurements;
};
} // namespace scopy::extprocplugin

#endif // RUNRESULTS_H
