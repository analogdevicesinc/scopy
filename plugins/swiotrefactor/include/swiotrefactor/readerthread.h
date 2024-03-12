/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef READERTHREAD_H
#define READERTHREAD_H

//#include "src/runtime/ad74413r/chnlinfo.h"

#include <iio.h>

#include <QMap>
#include <QMutex>
#include <QThread>

#include <atomic>
#include <iioutil/commandqueue.h>

namespace scopy::swiotrefactor {
class ReaderThread : public QThread
{
	Q_OBJECT
public:
	explicit ReaderThread(bool isBuffered, CommandQueue *cmdQueue, QObject *parent = nullptr);
	~ReaderThread();

	void addDioChannel(int index, struct iio_channel *channel);

	void createDioChannelCommand(int index);

	void addBufferedDevice(iio_device *device);

	void runDio();
	void singleDio();

	void runBuffered(int requiredBuffersNumber = 0);

	void createIioBuffer();

	void destroyIioBuffer();

	void cancelIioBuffer();

	void enableIioChnls();
	void initIioChannels();

	int getEnabledChnls();

	//	QVector<ChnlInfo *> getEnabledBufferedChnls();

	void startCapture(int requiredBuffersNumber = 0);

	void requestStop();
	void forcedStop();
public Q_SLOTS:
	void handleConnectionDestroyed();
	//	void onChnlsChange(QMap<int, ChnlInfo *> chnlsInfo);
	void onSamplingFrequencyComputed(double samplingFreq);

Q_SIGNALS:
	void readerThreadFinished();
	void bufferRefilled(QVector<QVector<double>> bufferData, int bufferCounter);
	void channelDataChanged(int channelId, double value);

private Q_SLOTS:
	void bufferRefillCommandFinished(scopy::Command *cmd);
	void bufferCreateCommandFinished(scopy::Command *cmd);
	void bufferDestroyCommandFinished(scopy::Command *cmd);
	void bufferDestroyCommandStarted(scopy::Command *cmd);
	void bufferCancelCommandFinished(scopy::Command *cmd);

private:
	void run() override;

	bool isBuffered;
	bool m_deinit;
	QMap<int, struct iio_channel *> m_dioChannels;

	double m_samplingFreq = 4800;
	int m_enabledChnlsNo;
	std::atomic<int> bufferCounter;
	std::atomic<int> m_requiredBuffersNumber;
	CommandQueue *m_cmdQueue;

	struct iio_device *m_iioDev;
	struct iio_buffer *m_iioBuff;
	//	QMap<int, ChnlInfo *> m_chnlsInfo;
	//	QVector<ChnlInfo *> m_bufferedChnls;
	QVector<QVector<double>> m_bufferData;
	std::atomic<bool> m_running, m_bufferInvalid;
	std::mutex m_mutex;
};
} // namespace scopy::swiotrefactor

#endif // READERTHREAD_H
