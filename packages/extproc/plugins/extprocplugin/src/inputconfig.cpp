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

#include "inputconfig.h"

#include <QFile>
#include <mapkeys.h>

using namespace scopy::extprocplugin;

InputConfig::InputConfig()
	: m_sampleCount(0)
	, m_channelCount(0)
	, m_samplingFrequency(0)
	, m_frequencyOffset(0.0)
{}

InputConfig::~InputConfig() {}

bool InputConfig::isValid()
{
	return m_sampleCount > 0 && m_channelCount > 0 && m_samplingFrequency > 0 && !m_inputFileFormat.isEmpty() &&
		!m_chnlsFormat.isEmpty() && QFile::exists(m_inputFile);
}

void InputConfig::fromVariantMap(const QVariantMap &params)
{
	m_sampleCount = params.value(KeysInputConfig::SAMPLE_COUNT, 0).toInt();
	m_channelCount = params.value(KeysInputConfig::CHANNEL_COUNT, 0).toInt();
	m_samplingFrequency = params.value(KeysInputConfig::SAMPLING_FREQUENCY, 0).toInt();
	m_frequencyOffset = params.value(KeysInputConfig::FREQUENCY_OFFSET, 0.0).toDouble();
	m_inputFile = params.value(KeysInputConfig::INPUT_FILE, "").toString();
	m_inputFileFormat = params.value(KeysInputConfig::INPUT_FILE_FORMAT, "").toString();
	m_chnlsFormat = params.value(KeysInputConfig::CHANNEL_FORMAT, {}).toStringList();
}

QVariantMap InputConfig::toVariantMap() const
{
	QVariantMap map;
	map[KeysInputConfig::SAMPLE_COUNT] = m_sampleCount;
	map[KeysInputConfig::CHANNEL_COUNT] = m_channelCount;
	map[KeysInputConfig::SAMPLING_FREQUENCY] = m_samplingFrequency;
	map[KeysInputConfig::FREQUENCY_OFFSET] = m_frequencyOffset;
	map[KeysInputConfig::INPUT_FILE] = m_inputFile;
	map[KeysInputConfig::INPUT_FILE_FORMAT] = m_inputFileFormat;
	map[KeysInputConfig::CHANNEL_FORMAT] = m_chnlsFormat;
	return map;
}

// Getters & Setters
int InputConfig::sampleCount() const { return m_sampleCount; }
void InputConfig::setSampleCount(int count) { m_sampleCount = count; }

int InputConfig::channelCount() const { return m_channelCount; }
void InputConfig::setChannelCount(int count) { m_channelCount = count; }

double InputConfig::samplingFrequency() const { return m_samplingFrequency; }
void InputConfig::setSamplingFrequency(double freq) { m_samplingFrequency = freq; }

double InputConfig::frequencyOffset() const { return m_frequencyOffset; }

void InputConfig::setFrequencyOffset(double offset) { m_frequencyOffset = offset; }

QString InputConfig::inputFileFormat() const { return m_inputFileFormat; }
void InputConfig::setInputFileFormat(const QString &format) { m_inputFileFormat = format; }

QString InputConfig::inputFile() const { return m_inputFile; }
void InputConfig::setInputFile(const QString &newInputFile) { m_inputFile = newInputFile; }

QStringList InputConfig::chnlsFormat() const { return m_chnlsFormat; }
void InputConfig::setChnlsFormat(const QStringList &format) { m_chnlsFormat = format; }
