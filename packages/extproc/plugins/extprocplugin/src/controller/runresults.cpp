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

#include "controller/runresults.h"

using namespace scopy::extprocplugin;

RunResults::RunResults()
{
	// TODO: Initialize members
}

bool RunResults::isValid()
{
	// TODO: Check if results are valid
	return true;
}

QVariantMap RunResults::getResultForAnalysis(QString type)
{
	// TODO: Get result for specific analysis type
	return QVariantMap();
}

QVariant RunResults::getMeasurement(QString name)
{
	// TODO: Get specific measurement by name
	return QVariant();
}

QString RunResults::getOutputFile() const { return m_outputFile; }

void RunResults::setOutputFile(const QString &outputFile) { m_outputFile = outputFile; }

QVariantMap RunResults::getResults() const { return m_results; }

void RunResults::setResults(const QVariantMap &results) { m_results = results; }

QVariantMap RunResults::getMeasurements() const { return m_measurements; }

void RunResults::setMeasurements(const QVariantMap &measurements) { m_measurements = measurements; }
