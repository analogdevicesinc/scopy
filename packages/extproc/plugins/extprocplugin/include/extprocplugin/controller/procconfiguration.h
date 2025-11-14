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

#ifndef PROCCONFIGURATION_H
#define PROCCONFIGURATION_H

#include <QObject>
#include <QVariantMap>
#include <QList>
#include <inputconfig.h>
#include <outputconfig.h>
#include <analysisconfig.h>

namespace scopy::extprocplugin {

class ProcConfiguration : public QObject
{
	Q_OBJECT
public:
	explicit ProcConfiguration(QObject *parent = nullptr);
	~ProcConfiguration();

	// Getters
	InputConfig getInputConfig() const;
	OutputConfig getOutputConfig() const;
	AnalysisConfig getAnalysisList() const;

	// Setters
	void setInputConfig(const InputConfig &inputConfig);
	void setOutputConfig(const OutputConfig &outputConfig);
	void setAnalysis(const AnalysisConfig &analysisList);

	// Utility methods
	void reset();
	bool isComplete() const;

private:
	InputConfig m_inputConfig;
	OutputConfig m_outputConfig;
	AnalysisConfig m_analysis;
};
} // namespace scopy::extprocplugin

#endif // PROCCONFIGURATION_H
