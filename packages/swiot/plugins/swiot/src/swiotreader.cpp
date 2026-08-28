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

#include "swiotreader.h"

#include "ad74413r/bufferlogic.h"
#include "swiot_logging_categories.h"

#include <algorithm>

#include <component/attribute.h>
#include <component/channel.h>
#include <component/device.h>
#include <component/streamview.h>
#include <component/backends/iio/iioinputstream.h>
#include <component/backends/iio/iioscanelement.h>

using namespace scopy;
using namespace scopy::swiot;

SwiotReader::SwiotReader(bool isBuffered, QObject *parent)
	: QObject{parent}
	, isBuffered(isBuffered)
	, m_inputStream(nullptr)
	, m_requiredBuffersNumber(0)
	, bufferCounter(0)
	, m_running(false)
{}

SwiotReader::~SwiotReader() {}

void SwiotReader::addDioChannel(int index, component::Channel *channel) { m_dioChannels.insert(index, channel); }

void SwiotReader::addBufferedDevice(component::Device *device)
{
	if(device) {
		m_inputStream = device->findChild<component::iio::IIOInputStream *>();
	}
}

QVector<ChnlInfo *> SwiotReader::getEnabledBufferedChnls()
{
	QVector<ChnlInfo *> enabledBufferedChnls = {};
	const auto keys = m_chnlsInfo.keys();
	for(const auto &key : keys) {
		if(m_chnlsInfo[key]->isScanElement() && m_chnlsInfo[key]->isEnabled() &&
		   !m_chnlsInfo[key]->isOutput()) {
			enabledBufferedChnls.push_back(m_chnlsInfo[key]);
		}
	}
	// StreamFormat channels are ordered by ascending scan-element index; match it.
	std::sort(enabledBufferedChnls.begin(), enabledBufferedChnls.end(),
		  [](ChnlInfo *a, ChnlInfo *b) { return a->scanIndex() < b->scanIndex(); });
	return enabledBufferedChnls;
}

void SwiotReader::onChnlsChange(QMap<int, ChnlInfo *> chnlsInfo) { m_chnlsInfo = chnlsInfo; }

void SwiotReader::onSamplingFrequencyComputed(double samplingFrequency) { m_samplingFreq = samplingFrequency; }

void SwiotReader::startCapture(int requiredBuffersNumber)
{
	if(isBuffered) {
		if(m_running) {
			return;
		}
		bufferCounter = 0;
		m_requiredBuffersNumber = requiredBuffersNumber;
		m_running = true;
		m_acqLoopTask = acquisitionLoop();
	} else {
		readDio();
	}
}

QCoro::Task<void> SwiotReader::acquisitionLoop()
{
	if(!m_inputStream) {
		m_running = false;
		Q_EMIT swiotReaderFinished();
		co_return;
	}

	m_bufferedChnls = getEnabledBufferedChnls();
	// Re-read scale/offset: the device function/range may have changed between captures.
	QList<int> enabledChnls{};
	for(ChnlInfo *chnl : std::as_const(m_bufferedChnls)) {
		chnl->readScaleOffset();
		enabledChnls.push_back(static_cast<int>(chnl->scanIndex()));
	}

	unsigned samplesCount = (m_samplingFreq >= MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : MIN_BUFFER_SIZE;
	if(!co_await m_inputStream->openAsync({enabledChnls, samplesCount})) {
		qWarning(CAT_SWIOT_AD74413R) << "Cannot open the input stream!";
		m_running = false;
		Q_EMIT swiotReaderFinished();
		co_return;
	}

	while(m_running) {
		if((m_requiredBuffersNumber != 0) && (bufferCounter >= m_requiredBuffersNumber)) {
			break;
		}

		auto r = co_await m_inputStream->refillAsync();
		if(!r) {
			qDebug(CAT_SWIOT_AD74413R) << "Refill error";
			break;
		}
		component::StreamView view(m_inputStream->readFormat());
		int chnlsNo = m_bufferedChnls.size();
		m_bufferData.clear();
		for(int c = 0; c < chnlsNo; c++) {
			QVector<double> &chnlData = m_bufferData[c];
			for(size_t s = 0; s < view.sampleCount(); s++) {
				// R4: preserve ChnlInfo::convertData bit-math; feed the raw
				// s16 container. Validate decoded values on hardware.
				uint16_t raw = view.raw<uint16_t>(c, s);
				chnlData.push_back(m_bufferedChnls[c]->convertData(raw));
			}
		}
		Q_EMIT bufferRefilled(m_bufferData, bufferCounter);
		bufferCounter++;
	}

	co_await m_inputStream->closeAsync();
	m_running = false;
	m_bufferedChnls.clear();
	Q_EMIT swiotReaderFinished();
}

void SwiotReader::readDio() { dioReadOnce(); }

void SwiotReader::singleDio() { dioReadOnce(); }

QCoro::Task<void> SwiotReader::dioReadOnce()
{
	const auto keys = m_dioChannels.keys();
	for(int index : keys) {
		component::Channel *chnl = m_dioChannels[index];
		if(!chnl) {
			continue;
		}
		component::Attribute *rawAttr =
			chnl->findChild<component::Attribute *>("raw", Qt::FindDirectChildrenOnly);
		if(!rawAttr || !rawAttr->readCapability()) {
			continue;
		}
		auto r = co_await rawAttr->readCapability()->readAsync();
		if(!r) {
			qCritical(CAT_SWIOT_MAX14906) << "Failed to acquire data on DIO channel" << index;
			continue;
		}
		bool ok = false;
		double raw = rawAttr->cachedValue().toDouble(&ok);
		if(ok) {
			Q_EMIT channelDataChanged(index, raw);
		}
	}
}

void SwiotReader::requestStop() { m_running = false; }

QCoro::Task<void> SwiotReader::forcedStop()
{
	m_running = false;
	if(m_acqLoopTask) {
		co_await m_acqLoopTask.value();
		m_acqLoopTask.reset();
	}
}

#include "moc_swiotreader.cpp"
