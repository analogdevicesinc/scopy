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

#include "ad74413r/bufferlogic.h"

#include "ad74413r/chnlinfobuilder.h"
#include "ad74413r/ad74413r.h"

#include <iioutil/iiocommand/iiochannelattributeread.h>
#include <iioutil/iiocommand/iiochannelattributewrite.h>
#include <iioutil/iiocommand/iiodeviceattributeread.h>

using namespace scopy::swiotrefactor;

BufferLogic::BufferLogic(QMap<QString, iio_device *> devicesMap, CommandQueue *commandQueue)
	: m_plotChnlsNo(0)
	, m_iioDevicesMap(devicesMap)
	, m_commandQueue(commandQueue)
{
	if(m_iioDevicesMap.contains(AD_NAME) && m_iioDevicesMap.contains(SWIOT_DEVICE_NAME)) {
		createChannels();
	}
}

BufferLogic::~BufferLogic()
{
	if(m_chnlsInfo.size() > 0) {
		qDeleteAll(m_chnlsInfo);
		m_chnlsInfo.clear();
	}
}

void BufferLogic::createChannels()
{
	if(m_iioDevicesMap[AD_NAME]) {
		int chnlsNumber = iio_device_get_channels_count(m_iioDevicesMap[AD_NAME]);
		int plotChnlsNo = 0;
		int chnlIdx = -1;
		const QRegExp rx("[^0-9]+");
		for(int i = 0; i < chnlsNumber; i++) {
			struct iio_channel *iioChnl = iio_device_get_channel(m_iioDevicesMap[AD_NAME], i);
			QString chnlId(iio_channel_get_id(iioChnl));
			QString chnlInfoId = chnlId[0].toLower();
			if(iio_channel_find_attr(iioChnl, "threshold")) {
				chnlInfoId = "d";
			}
			ChnlInfo *channelInfo = ChnlInfoBuilder::build(iioChnl, chnlInfoId, m_commandQueue);
			const auto &&parts = chnlId.split(rx);
			chnlIdx = -1;
			plotChnlsNo = (!channelInfo->isOutput() && channelInfo->isScanElement()) ? (plotChnlsNo + 1)
												 : plotChnlsNo;
			if(parts.size() <= 1) {
				continue;
			}
			if(parts[1].compare("")) {
				chnlIdx = parts[1].toInt();
				chnlIdx = (channelInfo->isOutput()) ? (chnlIdx + MAX_INPUT_CHNLS_NO) : chnlIdx;
			}
			m_chnlsInfo[chnlIdx] = channelInfo;
		}
		m_plotChnlsNo = plotChnlsNo;
	}
}

bool BufferLogic::verifyChannelsEnabledChanges(QVector<bool> enabledChnls)
{
	bool changes = false;
	for(int i = 0; i < enabledChnls.size(); i++) {
		if(m_chnlsInfo.contains(i)) {
			if(enabledChnls[i] != m_chnlsInfo[i]->isEnabled()) {
				changes = true;
				break;
			}
		}
	}
	return changes;
}

void BufferLogic::applyChannelsEnabledChanges(QVector<bool> enabledChnls)
{
	for(int i = 0; i < enabledChnls.size(); i++) {
		if(m_chnlsInfo.contains(i)) {
			if(enabledChnls[i] != m_chnlsInfo[i]->isEnabled()) {
				m_chnlsInfo[i]->setIsEnabled(enabledChnls[i]);
			}
		}
	}
	Q_EMIT chnlsChanged(m_chnlsInfo);
	computeSamplingFrequency();
}

void BufferLogic::applySamplingFrequencyChanges(int channelId, int value)
{
	if(m_samplingFrequencies.contains(channelId)) {
		m_samplingFrequencies[channelId] = value;
	} else {
		m_samplingFrequencies.insert(channelId, value);
	}
	computeSamplingFrequency();
}

void BufferLogic::computeSamplingFrequency()
{
	double newSamplingFrequency = 0.0;
	auto keys = m_samplingFrequencies.keys();
	for(int channelId : keys) {
		if(m_chnlsInfo[channelId]->isEnabled()) {
			newSamplingFrequency += (1.0 / m_samplingFrequencies[channelId]);
		}
	}
	newSamplingFrequency = (newSamplingFrequency != 0.0) ? (1.0 / newSamplingFrequency) : 1.0;
	m_samplingFrequency = newSamplingFrequency;
	Q_EMIT samplingFrequencyComputed(newSamplingFrequency);
}

int BufferLogic::getPlotChnlsNo() { return m_plotChnlsNo; }

QString BufferLogic::getPlotChnlUnitOfMeasure(int channel)
{
	QString unit = "";
	ChnlInfo *chnlInfo = m_chnlsInfo[channel];
	if(chnlInfo) {
		if(chnlInfo->isScanElement() && !chnlInfo->isOutput()) {
			unit = chnlInfo->unitOfMeasure();
		}
	}
	return unit;
}

QVector<QString> BufferLogic::getPlotChnlsUnitOfMeasure()
{
	QVector<QString> chnlsUnitOfMeasure;

	for(ChnlInfo *chnl : qAsConst(m_chnlsInfo)) {
		if(chnl->isScanElement() && !chnl->isOutput()) {
			QString unitOfMeasure = chnl->unitOfMeasure();
			chnlsUnitOfMeasure.push_back(unitOfMeasure);
		}
	}
	return chnlsUnitOfMeasure;
}

std::pair<int, int> BufferLogic::getPlotChnlRangeValues(int channel)
{
	std::pair<int, int> range = {0, 0};
	ChnlInfo *chnlInfo = m_chnlsInfo[channel];
	if(chnlInfo) {
		if(chnlInfo->isScanElement() && !chnlInfo->isOutput()) {
			range = chnlInfo->rangeValues();
		}
	}
	return range;
}

QVector<std::pair<int, int>> BufferLogic::getPlotChnlsRangeValues()
{
	QVector<std::pair<int, int>> chnlsRangeValues;
	for(ChnlInfo *chnl : qAsConst(m_chnlsInfo)) {
		if(chnl->isScanElement() && !chnl->isOutput()) {
			std::pair<int, int> rangeValues = chnl->rangeValues();
			chnlsRangeValues.push_back(rangeValues);
		}
	}
	return chnlsRangeValues;
}

std::pair<double, double> BufferLogic::getChnlOffsetScale(int channel)
{
	std::pair<double, double> offsetScale = {0, 1};
	ChnlInfo *chnlInfo = m_chnlsInfo.contains(channel) ? m_chnlsInfo[channel] : nullptr;
	if(chnlInfo) {
		offsetScale = chnlInfo->offsetScalePair();
	}
	return offsetScale;
}

QMap<int, QString> BufferLogic::getPlotChnlsId()
{
	QMap<int, QString> chnlsId;
	auto keys = m_chnlsInfo.keys();
	for(int key : keys) {
		if(m_chnlsInfo[key]->isScanElement() && !m_chnlsInfo[key]->isOutput()) {
			QString chnlId = m_chnlsInfo[key]->chnlId();
			chnlsId[key] = chnlId;
		}
	}

	return chnlsId;
}

void BufferLogic::initAd74413rChnlsFunctions()
{
	//	on the SWIOT board we have only 4 channels
	for(int i = 0; i < 4; ++i) {
		initChannelFunction(i);
	}
}

void BufferLogic::initDiagnosticChannels()
{
	//	The last 4 channels from context are always the diagnostic channels
	//	(they are not physically on the board)
	for(int i = 4; i < MAX_INPUT_CHNLS_NO; i++) {
		Q_EMIT channelFunctionDetermined(i, "diagnostic");
	}
}

void BufferLogic::initChannelFunction(unsigned int i)
{
	std::string chnlEnableAttribute = "ch" + std::to_string(i) + "_enable";
	Command *enabledChnCmd =
		new IioDeviceAttributeRead(m_iioDevicesMap[SWIOT_DEVICE_NAME], chnlEnableAttribute.c_str(), nullptr);
	connect(
		enabledChnCmd, &scopy::Command::finished, this,
		[=, this](scopy::Command *cmd) { enabledChnCmdFinished(i, cmd); }, Qt::QueuedConnection);
	m_commandQueue->enqueue(enabledChnCmd);
}

void BufferLogic::enabledChnCmdFinished(unsigned int i, scopy::Command *cmd)
{
	IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}

	if(tcmd->getReturnCode() >= 0) {
		char *result = tcmd->getResult();
		bool ok = false;
		bool enabled = QString(result).toInt(&ok);
		if(!ok) {
			return;
		}
		if(enabled) {
			std::string deviceAttributeName = "ch" + std::to_string(i) + "_device";
			Command *configuredDevCmd = new IioDeviceAttributeRead(m_iioDevicesMap[SWIOT_DEVICE_NAME],
									       deviceAttributeName.c_str(), nullptr);
			connect(
				configuredDevCmd, &scopy::Command::finished, this,
				[=, this](scopy::Command *cmd) { configuredDevCmdFinished(i, cmd); },
				Qt::QueuedConnection);
			m_commandQueue->enqueue(configuredDevCmd);

		} else {
			Q_EMIT channelFunctionDetermined(i, "no_config");
		}
	}
}

void BufferLogic::configuredDevCmdFinished(unsigned int i, scopy::Command *cmd)
{
	IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}

	if(tcmd->getReturnCode() >= 0) {
		char *result = tcmd->getResult();
		std::string device = std::string(result);
		if(device == "ad74413r") {
			std::string functionAttributeName = "ch" + std::to_string(i) + "_function";
			Command *chnFunctionCmd = new IioDeviceAttributeRead(m_iioDevicesMap[SWIOT_DEVICE_NAME],
									     functionAttributeName.c_str(), nullptr);
			connect(
				chnFunctionCmd, &scopy::Command::finished, this,
				[=, this](scopy::Command *cmd) { chnFunctionCmdFinished(i, cmd); },
				Qt::QueuedConnection);
			m_commandQueue->enqueue(chnFunctionCmd);

		} else {
			Q_EMIT channelFunctionDetermined(i, "no_config");
		}
	}
}

void BufferLogic::chnFunctionCmdFinished(unsigned int i, scopy::Command *cmd)
{
	IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}

	if(tcmd->getReturnCode() >= 0) {
		char *result = tcmd->getResult();
		QString function = QString(result);
		Q_EMIT channelFunctionDetermined(i, function);
	}
}

QMap<QString, iio_channel *> BufferLogic::getIioChnl(int chnlIdx)
{
	QMap<QString, iio_channel *> chnlsMap;
	int outputChblIdx = chnlIdx + MAX_INPUT_CHNLS_NO;

	if(m_chnlsInfo.contains(chnlIdx) && !m_chnlsInfo[chnlIdx]->isOutput()) {
		chnlsMap["input"] = m_chnlsInfo[chnlIdx]->iioChnl();
	}

	if(m_chnlsInfo.contains(outputChblIdx) && m_chnlsInfo[outputChblIdx]->isOutput()) {
		chnlsMap["output"] = m_chnlsInfo[outputChblIdx]->iioChnl();
	}

	return chnlsMap;
}
