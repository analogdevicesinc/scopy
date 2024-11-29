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


#include "bufferlogic.h"
#include "src/runtime/ad74413r/ad74413r.h"
#include "src/swiot_logging_categories.h"
#include "chnlinfobuilder.h"
#include <iioutil/iiocommand/iiochannelattributewrite.h>
#include <iioutil/iiocommand/iiochannelattributeread.h>
#include <iioutil/iiocommand/iiodeviceattributeread.h>

using namespace scopy::swiot;

BufferLogic::BufferLogic(QMap<QString, iio_device*> devicesMap, CommandQueue *commandQueue)
	: m_plotChnlsNo(0)
	, m_iioDevicesMap(devicesMap)
	, m_commandQueue(commandQueue)
{
	if (m_iioDevicesMap.contains(AD_NAME) && m_iioDevicesMap.contains(SWIOT_DEVICE_NAME)) {
		createChannels();
	}
}

BufferLogic::~BufferLogic()
{
	if (m_chnlsInfo.size() > 0) {
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
		for (int i = 0; i < chnlsNumber; i++) {
			struct iio_channel* iioChnl = iio_device_get_channel(m_iioDevicesMap[AD_NAME], i);
			QString chnlId(iio_channel_get_id(iioChnl));
			QString chnlName(iio_channel_get_name(iioChnl));
			if (chnlName == "fault") { continue; }
			ChnlInfo *channelInfo = ChnlInfoBuilder::build(iioChnl, chnlId[0].toLower(), m_commandQueue);
			const auto&& parts = chnlId.split(rx);
			chnlIdx = -1;
			plotChnlsNo = (!channelInfo->isOutput() && channelInfo->isScanElement()) ? (plotChnlsNo + 1) : plotChnlsNo;
			if (parts.size() <= 1) {
				continue;
			}
			if(parts[1].compare("")){
				chnlIdx = parts[1].toInt();
				chnlIdx = (channelInfo->isOutput()) ? (chnlIdx + MAX_INPUT_CHNLS_NO) : chnlIdx;
			}
			m_chnlsInfo[chnlIdx] = channelInfo;
		}
		m_plotChnlsNo = plotChnlsNo;
	}

}

bool BufferLogic::verifyEnableChanges(std::vector<bool> enabledChnls)
{
	bool changes = false;
	for (int i = 0; i < enabledChnls.size(); i++) {
		if (m_chnlsInfo.contains(i)) {
			if (enabledChnls[i] != m_chnlsInfo[i]->isEnabled()) {
				m_chnlsInfo[i]->setIsEnabled(enabledChnls[i]);
				changes = true;
			}
		}
	}
	if (changes) {
		Q_EMIT chnlsChanged(m_chnlsInfo);
	}
	return changes;
}

void BufferLogic::onSamplingFreqChanged(int idx)
{
	std::string newSamplingFreq = m_samplingFreqAvailable[idx].toStdString();

	if (!m_chnlsInfo.size()) {
		return;
	}
	Command *writeSrCmd = new IioChannelAttributeWrite(m_chnlsInfo.first()->iioChnl(), SAMPLING_FREQ_ATTR_NAME,
			newSamplingFreq.c_str(), nullptr);
	connect(writeSrCmd, &scopy::Command::finished, this, [=, this](scopy::Command *cmd) {
		IioChannelAttributeWrite *tcmd = dynamic_cast<IioChannelAttributeWrite*>(cmd);
		if (!tcmd) {
			return;
		}
		if (tcmd->getReturnCode() < 0) {
			qDebug(CAT_SWIOT_AD74413R) << "Chnl attribute write error " + QString::number(tcmd->getReturnCode());
		} else {
			readChnlsSamplingFreqAttr();
		}
	}, Qt::QueuedConnection);
	m_commandQueue->enqueue(writeSrCmd);
}

void BufferLogic::readChnlsSamplingFreqAvailableAttr()
{
	std::string s_attrName = "sampling_frequency_available";

	if (!m_chnlsInfo.size()) {
		return;
	}
	Command *readSrCommand = new IioChannelAttributeRead(m_chnlsInfo.first()->iioChnl(), s_attrName.c_str(), nullptr);
	connect(readSrCommand, &scopy::Command::finished,
		this, [=, this](scopy::Command* cmd) {
		IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead*>(cmd);
		if (!tcmd) {
			return;
		}
		QStringList attrValues;
		char *sr_available = tcmd->getResult();

		if (tcmd->getReturnCode() < 0) {
			qDebug(CAT_SWIOT_AD74413R) << "Chnl attribute read error " + QString::number(tcmd->getReturnCode());
		} else {
			QString bufferValues(sr_available);
			m_samplingFreqAvailable = bufferValues.split(" ");
			m_samplingFreqAvailable.removeAll("");
			Q_EMIT samplingFreqAvailableRead(m_samplingFreqAvailable);
		}
	}, Qt::QueuedConnection);
	m_commandQueue->enqueue(readSrCommand);
}

void BufferLogic::readChnlsSamplingFreqAttr()
{
	std::string s_attrName = "sampling_frequency";

	if (!m_chnlsInfo.size()) {
		return;
	}
	Command *readSrCommand = new IioChannelAttributeRead(m_chnlsInfo.first()->iioChnl(), s_attrName.c_str(), nullptr);
	connect(readSrCommand, &scopy::Command::finished,
		this, [=, this](scopy::Command* cmd) {
		IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead*>(cmd);
		if (!tcmd) {
			return;
		}
		QStringList attrValues;
		char *srAttrValueStr = tcmd->getResult();

		if (tcmd->getReturnCode() < 0) {
			qDebug(CAT_SWIOT_AD74413R) << "Chnl attribute read error " + QString::number(tcmd->getReturnCode());
		} else {
			try {
				int samplingFreq = std::stoi(srAttrValueStr);
				Q_EMIT samplingFreqRead(samplingFreq);
			} catch (std::invalid_argument& exception) {
				qDebug(CAT_SWIOT_AD74413R) << "Chnl attribute read error conversion: " + QString(srAttrValueStr);
			}
		}
	}, Qt::QueuedConnection);
	m_commandQueue->enqueue(readSrCommand);
}

int BufferLogic::getPlotChnlsNo()
{
	return m_plotChnlsNo;
}

QVector<QString> BufferLogic::getPlotChnlsUnitOfMeasure()
{
	QVector<QString> chnlsUnitOfMeasure;

	for (ChnlInfo* chnl : m_chnlsInfo) {
		if (chnl->isScanElement() && !chnl->isOutput()) {
			QString unitOfMeasure = chnl->unitOfMeasure();
			chnlsUnitOfMeasure.push_back(unitOfMeasure);
		}
	}
	return chnlsUnitOfMeasure;
}

QVector<std::pair<int, int>> BufferLogic::getPlotChnlsRangeValues()
{
	QVector<std::pair<int, int>> chnlsRangeValues;
	for (ChnlInfo* chnl : m_chnlsInfo) {
		if (chnl->isScanElement() && !chnl->isOutput()) {
			std::pair<int, int> rangeValues = chnl->rangeValues();
			chnlsRangeValues.push_back(rangeValues);
		}
	}
	return chnlsRangeValues;
}

QMap<int, QString> BufferLogic::getPlotChnlsId()
{
	QMap<int, QString> chnlsId;
	for (int key : m_chnlsInfo.keys()){
		if (m_chnlsInfo[key]->isScanElement() && !m_chnlsInfo[key]->isOutput()) {
			QString chnlId = m_chnlsInfo[key]->chnlId();
			chnlsId[key] = chnlId;
		}
	}

	return chnlsId;
}

void BufferLogic::initAd74413rChnlsFunctions()
{
//	on the SWIOT board we have only 4 channels
	for (int i = 0; i < 4; ++i) {
		initChannelFunction(i);
	}
}

void BufferLogic::initDiagnosticChannels()
{
	//	The last 4 channels from context are always the diagnostic channels
	//	(they are not physically on the board)
	for (int i = 4; i < MAX_INPUT_CHNLS_NO; i++) {
		Q_EMIT channelFunctionDetermined(i, "diagnostic");
	}
}

void BufferLogic::initChannelFunction(unsigned int i)
{
	std::string chnlEnableAttribute = "ch" + std::to_string(i) + "_enable";
	Command *enabledChnCmd = new IioDeviceAttributeRead(m_iioDevicesMap[SWIOT_DEVICE_NAME],
							    chnlEnableAttribute.c_str(), nullptr);
	connect(enabledChnCmd, &scopy::Command::finished, this, [=, this] (scopy::Command* cmd) {
		enabledChnCmdFinished(i, cmd);
	}, Qt::QueuedConnection);
	m_commandQueue->enqueue(enabledChnCmd);
}

void BufferLogic::enabledChnCmdFinished(unsigned int i, scopy::Command *cmd)
{
	IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead*>(cmd);
	if (!tcmd) { return; }

	if (tcmd->getReturnCode() >= 0) {
		char *result = tcmd->getResult();
		bool ok = false;
		bool enabled = QString(result).toInt(&ok);
		if (!ok) { return; }
		if (enabled) {
			std::string deviceAttributeName = "ch" + std::to_string(i) + "_device";
			Command *configuredDevCmd = new IioDeviceAttributeRead(m_iioDevicesMap[SWIOT_DEVICE_NAME],
									       deviceAttributeName.c_str(), nullptr);
			connect(configuredDevCmd, &scopy::Command::finished, this, [=, this] (scopy::Command* cmd) {
				configuredDevCmdFinished(i, cmd);
			}, Qt::QueuedConnection);
			m_commandQueue->enqueue(configuredDevCmd);

		} else {
			Q_EMIT channelFunctionDetermined(i, "no_config");
		}
	}
}

void BufferLogic::configuredDevCmdFinished(unsigned int i, scopy::Command *cmd)
{
	IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead*>(cmd);
	if (!tcmd) { return; }

	if (tcmd->getReturnCode() >= 0) {
		char *result = tcmd->getResult();
		std::string device = std::string(result);
		if (device == "ad74413r") {
			std::string functionAttributeName = "ch" + std::to_string(i) + "_function";
			Command *chnFunctionCmd = new IioDeviceAttributeRead(m_iioDevicesMap[SWIOT_DEVICE_NAME],
									     functionAttributeName.c_str(), nullptr);
			connect(chnFunctionCmd, &scopy::Command::finished, this, [=, this] (scopy::Command* cmd) {
				chnFunctionCmdFinished(i, cmd);
			}, Qt::QueuedConnection);
			m_commandQueue->enqueue(chnFunctionCmd);

		} else {
			Q_EMIT channelFunctionDetermined(i, "no_config");
		}
	}
}

void BufferLogic::chnFunctionCmdFinished(unsigned int i, scopy::Command *cmd)
{
	IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead*>(cmd);
	if (!tcmd) { return; }

	if (tcmd->getReturnCode() >= 0) {
		char *result = tcmd->getResult();
		QString function = QString(result);
		Q_EMIT channelFunctionDetermined(i, function);
	}
}

QMap<QString, iio_channel*> BufferLogic::getIioChnl(int chnlIdx)
{
	QMap<QString, iio_channel*> chnlsMap;
	int outputChblIdx = chnlIdx + MAX_INPUT_CHNLS_NO;

	if (m_chnlsInfo.contains(chnlIdx) && !m_chnlsInfo[chnlIdx]->isOutput()) {
		chnlsMap["input"] = m_chnlsInfo[chnlIdx]->iioChnl();
	}

	if (m_chnlsInfo.contains(outputChblIdx) && m_chnlsInfo[outputChblIdx]->isOutput()) {
		chnlsMap["output"] = m_chnlsInfo[outputChblIdx]->iioChnl();
	}

	return chnlsMap;
}


