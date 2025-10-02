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

#ifndef DATAREADER_H
#define DATAREADER_H

#include <QObject>
#include <QFile>
#include <QStringList>
#include "../extprocutils.h"

namespace scopy::extprocplugin {
class DataReader : public QObject
{
	Q_OBJECT

public:
	explicit DataReader(QObject *parent = nullptr);
	~DataReader();

	bool openFile(const QString &path);
	void unmap();

	void setChannelCount(int count);
	int channelCount() const;

	void setChannelFormat(const QStringList &format);
	QStringList channelFormat() const;

	int getBytesPerSample() const;
	int getBytesPerChannel(int channelIndex) const;

	void readData(int64_t startSample, int64_t sampleCount);

	void setChannelsName(const QStringList &newChannelsName);

	QStringList channelsName() const;

	// IQ binary file header support
	bool hasIQHeader() const;
	IQBinHeader getIQHeader() const;

	// Direct memory access to data section
	const uchar *mappedData() const;

Q_SIGNALS:
	void dataReady(QMap<QString, QVector<float>> &processedData);

private:
	int getFormatSize(const QString &format) const;
	float convertToFloat(const QByteArray &data, const QString &format) const;
	bool remapFile();
	bool checkForRemapping();
	void createFile(const QString &path);
	bool detectAndParseIQHeader(const uchar *mappedData, int64_t fileSize);
	int64_t getDataOffset() const;

	QFile m_file;
	uchar *m_data;
	int64_t m_dataSize;
	int m_channelCount;
	QStringList m_channelFormat;
	QStringList m_channelsName;
	QString m_filePath;

	// IQ binary header support
	bool m_hasIQHeader = false;
	IQBinHeader m_iqHeader = {};
	static const int HEADER_EXTRA_V2_SIZE = 1024;
};

} // namespace scopy::extprocplugin

#endif // DATAREADER_H
