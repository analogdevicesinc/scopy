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

#ifndef IIOMANAGER_H
#define IIOMANAGER_H

#include "extprocutils.h"

#include <iio.h>
#include <QObject>
#include <QFutureWatcher>
#include <datawriter.h>
#include <inputconfig.h>

namespace scopy::extprocplugin {

union iio_buffer_hack
{
	struct iio_buffer *buffer;
	struct
	{
		const struct iio_device *dev;
		void *buffer;
		char padding[40];
	} * fields;
};

class IIOManager : public QObject
{
	Q_OBJECT
public:
	IIOManager(iio_context *ctx, QObject *parent = nullptr);
	~IIOManager();

	QMap<QString, QList<ChannelInfo>> getAvailableChannels();

public Q_SLOTS:
	void startAcq(bool en);
	void onDataRequest();
	void onBufferParamsChanged(const BufferParams &params);

Q_SIGNALS:
	void inputFormatChanged(const InputConfig &config);
	void dataReady(QVector<QVector<double>> &inputData);

private:
	void computeDevMap();
	void destroyBuffer();
	void readBuffer();
	int enChannels(QString deviceName, QStringList enChnls);
	QStringList getChannelsFormat(iio_device *dev);
	double getSamplingFrequency(iio_device *dev);
	InputConfig createInputConfig(iio_device *dev, int channelCount, int64_t bufferSamplesSize);
	void chnlRead(iio_channel *chnl, QByteArray &dst);
	QVector<double> toDouble(QByteArray dst);
	void readAllChannels(QString deviceName);
	void updateBufferParams(const BufferParams &params);
	void notifyInputConfigChanged();

	iio_buffer *createMmapIioBuffer(struct iio_device *dev, size_t samples, void **originalBufferPtr = nullptr);

	int m_enChnlSize = 0;
	BufferParams m_params;
	iio_context *m_ctx;
	iio_buffer *m_buffer = nullptr;
	void *m_originalBufferPtr = nullptr;
	DataWriter *m_dataWriter;
	QFutureWatcher<void> *m_readFw;
	QVector<QVector<double>> m_bufferData;
	QMap<QString, QMap<QString, iio_channel *>> m_devMap;
};

} // namespace scopy::extprocplugin

#endif // IIOMANAGER_H
