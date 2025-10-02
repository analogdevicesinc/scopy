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
#include "plotmanager/datareader.h"

#include <QObject>
#include <QFile>
#include <QFutureWatcher>

namespace scopy::extprocplugin {

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
	void dataReady(QVector<QVector<float>> &inputData);

private:
	void readData();
	InputConfig createInputConfig();
	QVector<QVector<float>> convertIQData(const uchar *data, int numSamples);

	DataReader *m_dataReader;
	DataWriter *m_dataWriter;
	QFutureWatcher<void> *m_readFw;
	QVector<QVector<float>> m_bufferData;
	BufferParams m_params;
	bool m_isFileOpen = false;
	int64_t m_currentPosition = 0;
};

} // namespace scopy::extprocplugin

#endif // IQBINREADER_H
