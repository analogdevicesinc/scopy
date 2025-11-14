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

#include "outputconfig.h"

#include <QFile>
#include <mapkeys.h>

using namespace scopy::extprocplugin;

OutputConfig::OutputConfig() {}

OutputConfig::~OutputConfig() {}

bool OutputConfig::isValid() const { return !m_outputFile.isEmpty() && QFile::exists(m_outputFile); }

void OutputConfig::fromVariantMap(const QVariantMap &params)
{
	m_outputFile = params.value(KeysOutputConfig::OUTPUT_FILE, "").toString();
	m_outputFileFormat = params.value(KeysOutputConfig::OUTPUT_FILE_FORMAT, "").toString();
	m_enabledAnalysis = params.value(KeysOutputConfig::ENABLED_ANALYSIS, {}).toStringList();
}

QVariantMap OutputConfig::toVariantMap() const
{
	QVariantMap map;
	map[KeysOutputConfig::OUTPUT_FILE] = m_outputFile;
	map[KeysOutputConfig::OUTPUT_FILE_FORMAT] = m_outputFileFormat;
	map[KeysOutputConfig::ENABLED_ANALYSIS] = m_enabledAnalysis;
	return map;
}

QString OutputConfig::outputFile() const { return m_outputFile; }
void OutputConfig::setOutputFile(const QString &file) { m_outputFile = file; }

QString OutputConfig::outputFileFormat() const { return m_outputFileFormat; }
void OutputConfig::setOutputFileFormat(const QString &format) { m_outputFileFormat = format; }

QStringList OutputConfig::enabledAnalysis() const { return m_enabledAnalysis; }
void OutputConfig::setEnabledAnalysis(const QStringList &list) { m_enabledAnalysis = list; }
