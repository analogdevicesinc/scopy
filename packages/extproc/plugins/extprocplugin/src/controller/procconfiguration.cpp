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

#include "controller/procconfiguration.h"

using namespace scopy::extprocplugin;

ProcConfiguration::ProcConfiguration(QObject *parent)
	: QObject(parent)
	, m_inputConfig{}
	, m_outputConfig{}
	, m_analysis{}
{}

ProcConfiguration::~ProcConfiguration() {}

InputConfig ProcConfiguration::getInputConfig() const { return m_inputConfig; }

OutputConfig ProcConfiguration::getOutputConfig() const { return m_outputConfig; }

AnalysisConfig ProcConfiguration::getAnalysisList() const { return m_analysis; }

void ProcConfiguration::setInputConfig(const InputConfig &inputConfig) { m_inputConfig = inputConfig; }

void ProcConfiguration::setOutputConfig(const OutputConfig &outputConfig) { m_outputConfig = outputConfig; }

void ProcConfiguration::setAnalysis(const AnalysisConfig &analysis) { m_analysis = analysis; }

void ProcConfiguration::reset()
{
	m_inputConfig = {};
	m_outputConfig = {};
	m_analysis = {};
}

bool ProcConfiguration::isComplete() const
{
	QVariantMap input = m_inputConfig.toVariantMap();
	QVariantMap output = m_outputConfig.toVariantMap();
	return !input.isEmpty() && !output.isEmpty();
}
