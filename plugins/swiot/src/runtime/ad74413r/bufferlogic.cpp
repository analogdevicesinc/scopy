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

using namespace scopy::swiot;

BufferLogic::BufferLogic(QMap<QString, iio_device*> devicesMap) :
	m_plotChnlsNo(0)
      ,m_iioDevicesMap(devicesMap)
{
	if (m_iioDevicesMap.contains(AD_NAME) && m_iioDevicesMap.contains(SWIOT_DEVICE_NAME)) {
		initializeChnlsScaleInfo();
		createChannels();
		m_samplingFreqAvailable = readChnlsSamplingFreqAttr("sampling_frequency_available");
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
		bool isOutput = false;
		bool isScanElement = false;
		const QRegExp rx("[^0-9]+");
		for (int i = 0; i < chnlsNumber; i++) {
			struct iio_channel* iioChnl = iio_device_get_channel(m_iioDevicesMap[AD_NAME], i);
			struct chnlInfo* chnlInfo = new struct chnlInfo;
			QString chnlId(iio_channel_get_id(iioChnl));
			double offset = 0.0;
			double scale = 0.0;
			int erno = 0;
			const auto&& parts = chnlId.split(rx);
			chnlIdx = -1;
			iio_channel_disable(iioChnl);
			isOutput = iio_channel_is_output(iioChnl);
			erno = iio_channel_attr_read_double(iioChnl, "offset", &offset);
			iio_channel_attr_read_double(iioChnl, "scale", &scale);
			if (erno < 0) {
				scale = -1;
				offset = -1;
			}
			isScanElement = iio_channel_is_scan_element(iioChnl);
			plotChnlsNo = (!isOutput && isScanElement) ? (plotChnlsNo + 1) : plotChnlsNo;
			chnlInfo->chnlId = chnlId;
			chnlInfo->iioChnl = iioChnl;
			chnlInfo->isEnabled = false;
			chnlInfo->isScanElement = isScanElement;
			chnlInfo->isOutput = isOutput;
			chnlInfo->unitOfMeasure = m_unitsOfMeasure[chnlId[0].toLower()];
			chnlInfo->rangeValues = m_valuesRange[chnlId[0].toLower()];
			//to have the value in V and mA we need to multiply by 10^(-3)
			scale = (chnlInfo->unitOfMeasure == VOLTAGE_UM || chnlInfo->unitOfMeasure == CURRENT_UM)
					? (scale * 0.001) : scale;
			chnlInfo->offsetScalePair = {offset, scale};
			if (parts.size() > 1) {
				if(parts[1].compare("")){
					chnlIdx = parts[1].toInt();
					chnlIdx = (isOutput) ? (chnlIdx + MAX_INPUT_CHNLS_NO) : chnlIdx;
				}
			}
			m_chnlsInfo[chnlIdx] = chnlInfo;
		}
		m_plotChnlsNo = plotChnlsNo;
	}

}

void BufferLogic::initializeChnlsScaleInfo()
{
	m_unitsOfMeasure['v'] = VOLTAGE_UM;
	m_unitsOfMeasure['c'] = CURRENT_UM;
	m_unitsOfMeasure['r'] = RESISTANCE_UM;

	m_valuesRange['v'] = {-VOLTAGE_LIMIT, VOLTAGE_LIMIT};
	m_valuesRange['c'] = {-CURRENT_LIMIT, CURRENT_LIMIT};
	m_valuesRange['r'] = {RESISTANCE_LOWER_LIMIT, RESISTANCE_UPPER_LIMIT};
}

struct iio_channel* BufferLogic::getIioChnl(int chnlIdx, bool outputPriority)
{
	struct iio_channel* iioChnl = nullptr;
	if (outputPriority) {
		iioChnl = (m_chnlsInfo.contains(chnlIdx + MAX_INPUT_CHNLS_NO)) ?
					m_chnlsInfo[chnlIdx + MAX_INPUT_CHNLS_NO]->iioChnl : m_chnlsInfo[chnlIdx]->iioChnl;
	} else {
		iioChnl = m_chnlsInfo[chnlIdx]->iioChnl;
	}

	return iioChnl;
}

bool BufferLogic::verifyEnableChanges(std::vector<bool> enabledChnls)
{
	bool changes = false;
	for (int i = 0; i < enabledChnls.size(); i++) {
		if (m_chnlsInfo.contains(i)) {
			if (enabledChnls[i] != m_chnlsInfo[i]->isEnabled) {
				m_chnlsInfo[i]->isEnabled = enabledChnls[i] ;
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

	for (int key : m_chnlsInfo.keys()) {
		if (key > 0 && !m_chnlsInfo[key]->isOutput) {
			int returnCode = iio_channel_attr_write(m_chnlsInfo[key]->iioChnl, SAMPLING_FREQ_ATTR_NAME, newSamplingFreq.c_str());
			if (returnCode < 0) {
				qDebug(CAT_SWIOT_AD74413R) << "Chnl attribute write error " + QString::number(returnCode);
			} else {
				QStringList newSamplingFreq = readChnlsSamplingFreqAttr(SAMPLING_FREQ_ATTR_NAME);
				int samplingFreq = newSamplingFreq[0].toInt();
				Q_EMIT samplingFreqWritten(samplingFreq);
				break;
			}
		}
	}
}

QStringList BufferLogic::readChnlsSamplingFreqAttr(QString attrName)
{
	QStringList attrValues;
	char* buffer = new char[200];
	std::string s_attrName = attrName.toStdString();

	for (int key : m_chnlsInfo.keys()) {
		if (key > 0 && !m_chnlsInfo[key]->isOutput) {
			int returnCode = iio_channel_attr_read(m_chnlsInfo[key]->iioChnl, s_attrName.c_str(), buffer, 199);
			if (returnCode > 0) {
				QString bufferValues(buffer);
				attrValues = bufferValues.split(" ");
				break;
			} else {
				qDebug(CAT_SWIOT_AD74413R) << "Chnl attribute read error " + QString::number(returnCode);
			}
		}
	}
	attrValues.removeAll("");
	delete[] buffer;
	return attrValues;
}

int BufferLogic::getPlotChnlsNo()
{
	return m_plotChnlsNo;
}

QVector<QString> BufferLogic::getPlotChnlsUnitOfMeasure()
{
	QVector<QString> chnlsUnitOfMeasure;

	for (struct chnlInfo* chnl : m_chnlsInfo) {
		if (chnl->isScanElement && !chnl->isOutput) {
			QString unitOfMeasure = chnl->unitOfMeasure;
			chnlsUnitOfMeasure.push_back(unitOfMeasure);
		}
	}
	return chnlsUnitOfMeasure;
}

QVector<std::pair<int, int>> BufferLogic::getPlotChnlsRangeValues()
{
	QVector<std::pair<int, int>> chnlsRangeValues;
	for (struct chnlInfo* chnl : m_chnlsInfo) {
		if (chnl->isScanElement && !chnl->isOutput) {
			std::pair<int, int> rangeValues = chnl->rangeValues;
			chnlsRangeValues.push_back(rangeValues);
		}
	}
	return chnlsRangeValues;
}

QMap<int, QString> BufferLogic::getPlotChnlsId()
{
	QMap<int, QString> chnlsId;

	for (int key : m_chnlsInfo.keys()){
		if (m_chnlsInfo[key]->isScanElement && !m_chnlsInfo[key]->isOutput) {
			QString chnlId = m_chnlsInfo[key]->chnlId;
			chnlsId[key] = chnlId;
		}
	}

	return chnlsId;
}

QVector<QString> BufferLogic::getAd74413rChnlsFunctions() {
	auto result = QVector<QString>();
//	on the SWIOT board we have only 4 channels
	for (int i = 0; i < 4; ++i) {
		char device[256] = {0};
		bool isEnable = false;
		std::string deviceAttributeName = "ch" + std::to_string(i) + "_device";
		std::string chnlEnableAttribute = "ch" + std::to_string(i) + "_enable";
		ssize_t deviceReadResult = iio_device_attr_read(m_iioDevicesMap[SWIOT_DEVICE_NAME], deviceAttributeName.c_str(), device, 255);
		ssize_t chnlEnableResult = iio_device_attr_read_bool(m_iioDevicesMap[SWIOT_DEVICE_NAME], chnlEnableAttribute.c_str(), &isEnable);
		if (deviceReadResult > 0 && chnlEnableResult >= 0) {
			if (!isEnable || (strcmp(device, "max14906") == 0)) {
				result.push_back("no_config");
			} else {
				char function[256] = {0};
				std::string functionAttributeName = "ch" + std::to_string(i) + "_function";
				ssize_t functionReadResult = iio_device_attr_read(m_iioDevicesMap[SWIOT_DEVICE_NAME], functionAttributeName.c_str(), function, 255);
				if (functionReadResult > 0) {
					result.push_back(QString(function));
				}
			}
		}
	}
//	The last 4 channels from context are always the diagnostic channels
//	(they are not physically on the board)
	for (int i = result.size(); i < MAX_INPUT_CHNLS_NO; i++) {
		result.push_back("diagnostic");
	}

	return result;
}


