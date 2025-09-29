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

#ifndef IQBINREADER_H
#define IQBINREADER_H

#include "extprocutils.h"
#include "inputconfig.h"
#include "datawriter.h"

#include <QObject>
#include <QFile>
#include <QFutureWatcher>

namespace scopy::extprocplugin {

struct IQBinHeader
{
	int version;
	int num_points;
	double sample_rate;
	double start_time;
	double center_freq;
};

class IQBinReader : public QObject
{
	Q_OBJECT
public:
	IQBinReader(QObject *parent = nullptr);
	~IQBinReader();

	bool openFile(const QString &filePath);
	void closeFile();
	bool isFileOpen() const;

public Q_SLOTS:
	void startAcq(bool en);
	void onDataRequest();
	void onBufferParamsChanged(const BufferParams &params);

Q_SIGNALS:
	void inputFormatChanged(const InputConfig &config);
	void dataReady(QVector<QVector<double>> &inputData);

private:
	bool parseHeader();
	void readData();
	InputConfig createInputConfig();
	QVector<QVector<double>> convertIQData(const QByteArray &rawData, int numSamples);

	QFile m_file;
	IQBinHeader m_header;
	QFutureWatcher<void> *m_readFw;
	DataWriter *m_dataWriter;
	QVector<QVector<double>> m_bufferData;
	BufferParams m_params;
	bool m_isFileOpen = false;
	int64_t m_dataStartOffset = 0;
	int64_t m_currentPosition = 0;
	static const int HEADER_EXTRA_V2_SIZE = 1024;
};

} // namespace scopy::extprocplugin

#endif // IQBINREADER_H
