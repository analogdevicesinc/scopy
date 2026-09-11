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

#ifndef SWIOTREADER_H
#define SWIOTREADER_H

#include "ad74413r/chnlinfo.h"

#include <QCloseEvent>
#include <QMap>
#include <QObject>
#include <QVector>

#include <atomic>
#include <qcoro/qcorotask.h>

namespace scopy {
namespace component {
class Device;
class Channel;
namespace iio {
class IIOInputStream;
}
} // namespace component

namespace swiot {
// Despite the historical name, this is no longer a QThread: the pooled command
// executor already runs IIO commands on a worker pool, so acquisition is driven
// by a non-blocking QCoro coroutine loop on the owning (GUI) thread.
class SwiotReader : public QObject
{
	Q_OBJECT
public:
	explicit SwiotReader(bool isBuffered, QObject *parent = nullptr);
	~SwiotReader();

	void addDioChannel(int index, component::Channel *channel);

	void addBufferedDevice(component::Device *device);

	// DIO: read the "raw" attribute of every registered channel once (async).
	void readDio();
	void singleDio();

	QVector<ChnlInfo *> getEnabledBufferedChnls();

	void startCapture(int requiredBuffersNumber = 0);

	void requestStop();
	QCoro::Task<void> forcedStop();

	bool isRunning() const { return m_running; }

public Q_SLOTS:
	void onChnlsChange(QMap<int, ChnlInfo *> chnlsInfo);
	void onSamplingFrequencyComputed(double samplingFreq);

Q_SIGNALS:
	void swiotReaderFinished();
	void bufferRefilled(QMap<int, QVector<double>> bufferData, int bufferCounter);
	void channelDataChanged(int channelId, double value);

private:
	QCoro::Task<void> acquisitionLoop();
	QCoro::Task<void> dioReadOnce();

	bool isBuffered;
	QMap<int, component::Channel *> m_dioChannels;

	double m_samplingFreq = 4800;
	std::atomic<int> bufferCounter;
	std::atomic<int> m_requiredBuffersNumber;

	component::iio::IIOInputStream *m_inputStream;
	QMap<int, ChnlInfo *> m_chnlsInfo;
	QVector<ChnlInfo *> m_bufferedChnls;
	QMap<int, QVector<double>> m_bufferData;
	std::atomic<bool> m_running;
	std::optional<QCoro::Task<void>> m_acqLoopTask;
};
} // namespace swiot
} // namespace scopy

#endif // SWIOTREADER_H
