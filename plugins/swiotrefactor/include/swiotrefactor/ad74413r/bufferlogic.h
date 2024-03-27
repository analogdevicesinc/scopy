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

#ifndef BUFFERLOGIC_H
#define BUFFERLOGIC_H

#include "chnlinfo.h"

#include <iio.h>

#include <QMap>
#include <QObject>

#include <cerrno>
#include <iioutil/command.h>
#include <iioutil/commandqueue.h>

Q_DECLARE_OPAQUE_POINTER(struct iio_buffer *)

#define MAX_BUFFER_SIZE 160
#define MIN_BUFFER_SIZE 5
#define SAMPLING_FREQ_ATTR_NAME "sampling_frequency"
#define MAX_INPUT_CHNLS_NO 8

namespace scopy::swiotrefactor {
class BufferLogic : public QObject
{
	Q_OBJECT
public:
	explicit BufferLogic(QMap<QString, iio_device *> devicesMap, CommandQueue *commandQueue);

	~BufferLogic();

	QMap<QString, iio_channel *> getIioChnl(int chnlIdx);

	bool verifyChannelsEnabledChanges(QVector<bool> enabledChnls);
	void applyChannelsEnabledChanges(QVector<bool> enabledChnls);

	void applySamplingFrequencyChanges(int channelId, int value);

	int getPlotChnlsNo();
	QString getPlotChnlUnitOfMeasure(int channel);
	QVector<QString> getPlotChnlsUnitOfMeasure();
	std::pair<int, int> getPlotChnlRangeValues(int channel);
	QVector<std::pair<int, int>> getPlotChnlsRangeValues();
	std::pair<double, double> getChnlOffsetScale(int channel);
	QMap<int, QString> getPlotChnlsId();
	void initAd74413rChnlsFunctions();
	void initDiagnosticChannels();

Q_SIGNALS:
	void chnlsChanged(QMap<int, swiotrefactor::ChnlInfo *> chnlsInfo);
	void samplingFrequencyComputed(double value);
	void channelFunctionDetermined(unsigned int i, QString function);
	void instantValueChanged(int channel, double value);

private Q_SLOTS:
	void enabledChnCmdFinished(unsigned int i, scopy::Command *cmd);
	void configuredDevCmdFinished(unsigned int i, scopy::Command *cmd);
	void chnFunctionCmdFinished(unsigned int i, scopy::Command *cmd);

private:
	void createChannels();
	void initChannelFunction(unsigned int i);
	void computeSamplingFrequency();

private:
	int m_plotChnlsNo;
	QMap<QString, iio_device *> m_iioDevicesMap;
	QMap<int, int> m_samplingFrequencies;
	double m_samplingFrequency;

	QMap<int, ChnlInfo *> m_chnlsInfo;
	CommandQueue *m_commandQueue;
};
} // namespace scopy::swiotrefactor

#endif // BUFFERLOGIC_H
