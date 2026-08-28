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

#include <QMap>
#include <QObject>

namespace scopy {
namespace component {
class Device;
}

namespace swiot {

#define MAX_BUFFER_SIZE 160
#define MIN_BUFFER_SIZE 5
#define SAMPLING_FREQ_ATTR_NAME "sampling_frequency"
#define MAX_INPUT_CHNLS_NO 8

class BufferLogic : public QObject
{
	Q_OBJECT
public:
	explicit BufferLogic(component::Device *adDevice, component::Device *swiotDevice, QObject *parent = nullptr);

	~BufferLogic();

	QMap<QString, component::Channel *> getChnl(int chnlIdx);

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
	void chnlsChanged(QMap<int, swiot::ChnlInfo *> chnlsInfo);
	void samplingFrequencyComputed(double value);
	void channelFunctionDetermined(unsigned int i, QString function);
	void instantValueChanged(int channel, double value);

private:
	void createChannels();
	void initChannelFunction(unsigned int i);
	void computeSamplingFrequency();

private:
	int m_plotChnlsNo;
	component::Device *m_adDevice;
	component::Device *m_swiotDevice;
	QMap<int, int> m_samplingFrequencies;
	double m_samplingFrequency;

	QMap<int, ChnlInfo *> m_chnlsInfo;
};
} // namespace swiot
} // namespace scopy

#endif // BUFFERLOGIC_H
