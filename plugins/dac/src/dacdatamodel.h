/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef DACDATAMODEL_H
#define DACDATAMODEL_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QFuture>

#include <iio.h>

namespace scopy {
namespace dac {
class TxNode;
class DacDataModel : public QObject
{
	Q_OBJECT
public:
	DacDataModel(struct iio_device *dev, QObject *parent = nullptr);
	virtual ~DacDataModel();

	QString getName() const;
	bool isBufferCapable() const;
	bool isDds() const;

	void enableDds(bool enable);
	void enableBuffer(bool enabled);

	QMap<QString, TxNode *> getBufferTxs() const;
	QMap<QString, TxNode *> getDdsTxs() const;

	void setCyclic(bool cyclic);
	void setKernelBuffersCount(unsigned int kernelCount);
	void setDecimation(double decimation);
	void setBuffersize(unsigned int buffersize);
	void setFilesize(unsigned int filesize);
	void setData(QVector<QVector<int16_t>> data);
	void setSamplingFrequency(unsigned int sr);

	void enableBufferChannel(QString uuid, bool enable);
	void start();
	void stop();
	struct iio_device *getDev() const;
Q_SIGNALS:
	void reqInitBuffer();
	void log(QString log);
	void invalidRunParams();
	void updateBuffersize(unsigned int bf);
	void updateKernelBuffers(unsigned int kb);

public Q_SLOTS:
	void reset();

private Q_SLOTS:
	void initBuffer();

private:
	struct iio_device *m_dev;
	QString m_name;
	QList<struct iio_channel *> m_channels;
	struct iio_buffer *m_buffer;

	unsigned int m_buffersize;
	unsigned int m_userBuffersize;
	unsigned int m_filesize;
	unsigned int m_kernelBufferCount;
	unsigned int m_userKernelBufferCount;
	unsigned int m_samplingFrequency;
	bool m_isBufferCapable;
	bool m_isDds;
	bool m_cyclicBuffer;
	int m_decimation;

	bool m_activeDds;
	bool m_activeBuffer;

	const QString toneId = "altvoltage";
	const int MAX_NB_TONES = 4;
	const QString Q_CHANNEL = "Q";
	const QString I_CHANNEL = "I";

	QMap<QString, TxNode *> m_ddsTxs;
	QMap<QString, TxNode *> m_bufferTxs;

	QVector<QVector<int16_t>> m_data;
	QFuture<void> m_pushThd;
	bool m_interrupted;

	bool initBufferDac();
	bool initDdsDac();
	void deinitBufferDac();
	void deinitDdsDac();
	QString generateToneName(QString chnId);
	QStringList generateTxNodesForChannel(QString name);
	void push();
	unsigned int getEnabledChannelsCount();
	bool validateBufferParams();
	void requestInterruption();
	bool dmaValidSelection(unsigned mask, unsigned channel_count);
	bool txChannelsCheckValidSetup();
	int getTxChannelEnabledCount(unsigned *enabled_mask);
	void autoBuffersizeAndKernelBuffers();
};
} // namespace dac
} // namespace scopy

#endif // DACDATAMODEL_H
