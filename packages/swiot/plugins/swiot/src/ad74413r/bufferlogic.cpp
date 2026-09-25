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

#include <QRegularExpression>

#include "ad74413r/chnlinfobuilder.h"

#include <component/attribute.h>
#include <component/channel.h>
#include <component/device.h>
#include <component/backends/iio/iioinputstream.h>
#include <component/backends/iio/iioscanelement.h>

using namespace scopy;
using namespace scopy::swiot;

BufferLogic::BufferLogic(component::Device *adDevice, component::Device *swiotDevice, QObject *parent)
	: QObject(parent)
	, m_plotChnlsNo(0)
	, m_adDevice(adDevice)
	, m_swiotDevice(swiotDevice)
{
	if(m_adDevice && m_swiotDevice) {
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
	// Resolve which channels are scan elements (and at which index) from the
	// device's input stream.
	QMap<QString, long> scanElemIndex;
	component::iio::IIOInputStream *stream = m_adDevice->findChild<component::iio::IIOInputStream *>();
	if(stream) {
		const QList<component::iio::IIOScanElement *> elems =
			stream->findChildren<component::iio::IIOScanElement *>(Qt::FindDirectChildrenOnly);
		for(component::iio::IIOScanElement *el : elems) {
			scanElemIndex[el->id()] = el->index();
		}
	}

	int plotChnlsNo = 0;
	int chnlIdx = -1;
	const QRegularExpression rx("[^0-9]+");
	const QList<component::Channel *> chnls =
		m_adDevice->findChildren<component::Channel *>(Qt::FindDirectChildrenOnly);
	for(component::Channel *chnl : chnls) {
		QString chnlId = chnl->id();
		QString chnlInfoId = chnlId[0].toLower();
		if(chnl->findChild<component::Attribute *>("threshold", Qt::FindDirectChildrenOnly)) {
			chnlInfoId = "d";
		}
		ChnlInfo *channelInfo = ChnlInfoBuilder::build(chnl, chnlInfoId);
		if(scanElemIndex.contains(chnlId)) {
			channelInfo->setScanIndex(scanElemIndex[chnlId]);
		}
		const auto &&parts = chnlId.split(rx);
		chnlIdx = -1;
		plotChnlsNo =
			(!channelInfo->isOutput() && channelInfo->isScanElement()) ? (plotChnlsNo + 1) : plotChnlsNo;
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

	for(ChnlInfo *chnl : std::as_const(m_chnlsInfo)) {
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
	for(ChnlInfo *chnl : std::as_const(m_chnlsInfo)) {
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
	auto readAttr = [this](const QString &name) -> QString {
		component::Attribute *attr =
			m_swiotDevice->findChild<component::Attribute *>(name, Qt::FindDirectChildrenOnly);
		if(!attr || !attr->readCapability()) {
			return QString();
		}
		QCoro::waitFor(attr->readCapability()->readAsync());
		return attr->cachedValue();
	};

	bool ok = false;
	bool enabled = readAttr("ch" + QString::number(i) + "_enable").toInt(&ok);
	if(!ok || !enabled) {
		Q_EMIT channelFunctionDetermined(i, "no_config");
		return;
	}
	QString device = readAttr("ch" + QString::number(i) + "_device");
	if(device != "ad74413r") {
		Q_EMIT channelFunctionDetermined(i, "no_config");
		return;
	}
	QString function = readAttr("ch" + QString::number(i) + "_function");
	Q_EMIT channelFunctionDetermined(i, function);
}

QMap<QString, component::Channel *> BufferLogic::getChnl(int chnlIdx)
{
	QMap<QString, component::Channel *> chnlsMap;
	int outputChblIdx = chnlIdx + MAX_INPUT_CHNLS_NO;

	if(m_chnlsInfo.contains(chnlIdx) && !m_chnlsInfo[chnlIdx]->isOutput()) {
		chnlsMap["input"] = m_chnlsInfo[chnlIdx]->chnl();
	}

	if(m_chnlsInfo.contains(outputChblIdx) && m_chnlsInfo[outputChblIdx]->isOutput()) {
		chnlsMap["output"] = m_chnlsInfo[outputChblIdx]->chnl();
	}

	return chnlsMap;
}

#include "moc_bufferlogic.cpp"
