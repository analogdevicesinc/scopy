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

#include "analysisconfig.h"
#include <mapkeys.h>

using namespace scopy::extprocplugin;

AnalysisConfig::AnalysisConfig() {}

AnalysisConfig::~AnalysisConfig() {}

bool AnalysisConfig::isValid() const { return !m_analysisType.isEmpty(); }

void AnalysisConfig::fromVariantMap(const QVariantMap &params)
{
	m_analysisType = params.value(KeysAnalysisConfig::ANALYSIS_TYPE, "").toString();
	m_outInfo.fromVariantMap(params.value(KeysAnalysisConfig::OUT_INFO, {}).toMap());
	m_params = params.value(KeysAnalysisConfig::PARAMS, {}).toMap();
}

QVariantMap AnalysisConfig::toVariantMap() const
{
	QVariantMap map;
	map[KeysAnalysisConfig::ANALYSIS_TYPE] = m_analysisType;
	map[KeysAnalysisConfig::OUT_INFO] = m_outInfo.toVariantMap();
	map[KeysAnalysisConfig::PARAMS] = m_params;
	return map;
}

QString AnalysisConfig::analysisType() const { return m_analysisType; }
void AnalysisConfig::setAnalysisType(const QString &type) { m_analysisType = type; }

OutputInfo AnalysisConfig::outputInfo() const { return m_outInfo; }
void AnalysisConfig::setOutputInfo(const OutputInfo &info) { m_outInfo = info; }

QVariantMap AnalysisConfig::params() const { return m_params; }
void AnalysisConfig::setParams(const QVariantMap &params) { m_params = params; }
