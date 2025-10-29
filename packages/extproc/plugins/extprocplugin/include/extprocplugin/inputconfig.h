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

#ifndef INPUTCONFIG_H
#define INPUTCONFIG_H

#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace scopy::extprocplugin {

class InputConfig
{

public:
	InputConfig();
	~InputConfig();

	bool isValid();
	void fromVariantMap(const QVariantMap &params);
	QVariantMap toVariantMap() const;

	int sampleCount() const;
	void setSampleCount(int newSampleCount);

	int channelCount() const;
	void setChannelCount(int newChannelCount);

	double samplingFrequency() const;
	void setSamplingFrequency(double newSamplingFrequency);

	double frequencyOffset() const;
	void setFrequencyOffset(double newFrequencyOffset);

	QString inputFileFormat() const;
	void setInputFileFormat(const QString &newInputFileFormat);

	QStringList chnlsFormat() const;
	void setChnlsFormat(const QStringList &newChnlsFormat);

	QString inputFile() const;
	void setInputFile(const QString &newInputFile);

private:
	int m_sampleCount;
	int m_channelCount;
	double m_samplingFrequency;
	double m_frequencyOffset;
	QString m_inputFile;
	QString m_inputFileFormat;
	QStringList m_chnlsFormat;
};

} // namespace scopy::extprocplugin

#endif // INPUTCONFIG_H
