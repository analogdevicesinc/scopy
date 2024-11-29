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

#include "acquisitionmanager.h"
#include "qtconcurrentrun.h"
#include <QLoggingCategory>
#include <QTimer>

Q_LOGGING_CATEGORY(CAT_PQM_ACQ, "PqmAqcManager");
using namespace scopy::pqm;

AcquisitionManager::AcquisitionManager(iio_context *ctx, PingTask *pingTask, QObject *parent)
	: QObject(parent)
	, m_ctx(ctx)
	, m_pingTask(pingTask)
	, m_buffer(nullptr)
	, m_pqmLog(nullptr)
{
	m_readFw = new QFutureWatcher<void>(this);
	m_setFw = new QFutureWatcher<void>(this);
	iio_device *dev = iio_context_find_device(m_ctx, DEVICE_PQM);
	if(dev) {
		// might need to set a trigger for the pqm device
		m_pqmLog = new PqmDataLogger(this);
		m_hasFwVers = iio_device_find_attr(dev, "fw_version");
		readPqmAttributes();
		enableBufferChnls(dev);
		m_buffer = iio_device_create_buffer(dev, BUFFER_SIZE, false);
		if(!m_buffer) {
			qWarning(CAT_PQM_ACQ) << "Cannot create the buffer!";
		}
		m_pingTimer = new QTimer(this);
		m_pingTimer->setInterval(3000);
		connect(m_pingTimer, &QTimer::timeout, this, &AcquisitionManager::pingTimerTimeout);
		connect(m_readFw, &QFutureWatcher<void>::finished, this, &AcquisitionManager::onReadFinished,
			Qt::QueuedConnection);
		connect(this, &AcquisitionManager::logData, m_pqmLog, &PqmDataLogger::logPressed);
	} else {
		qWarning(CAT_PQM_ACQ) << "The PQM device is not available!";
	}
}

AcquisitionManager::~AcquisitionManager()
{
	if(m_pingTimer) {
		m_pingTimer->stop();
		m_pingTimer->deleteLater();
		m_pingTimer = nullptr;
	}
	if(m_readFw) {
		m_readFw->waitForFinished();
		m_readFw->deleteLater();
		m_readFw = nullptr;
	}
	if(m_setFw) {
		m_setFw->waitForFinished();
		m_setFw->deleteLater();
		m_setFw = nullptr;
	}
	if(m_ctx) {
		m_ctx = nullptr;
	}
	m_pingTask = nullptr;
	// buffer destroy?
	m_chnlsName.clear();
	m_bufferData.clear();
	m_pqmAttr.clear();
}

void AcquisitionManager::enableBufferChnls(iio_device *dev)
{
	int chnlsNo = iio_device_get_channels_count(dev);
	for(int i = 0; i < chnlsNo; i++) {
		iio_channel *chnl = iio_device_get_channel(dev, i);
		iio_channel_enable(chnl);
		QString chName(iio_channel_get_name(chnl));
		m_chnlsName.push_back(chName);
	}
	m_pqmLog->setChnlsName(m_chnlsName);
}

void AcquisitionManager::toolEnabled(bool en, QString toolName)
{
	m_tools[toolName] = en;
	QMap<QString, bool>::const_iterator it = std::find(m_tools.cbegin(), m_tools.cend(), true);
	if(it != m_tools.cend()) {
		stopPing();
		storeProcessData();
		if(!m_readFw->isRunning()) {
			futureReadData();
		}
	} else {
		m_readFw->waitForFinished();
		m_readFw->cancel();
		startPing();
	}
}

void AcquisitionManager::futureReadData()
{
	if(!m_readFw->isRunning()) {
		QFuture<void> f = QtConcurrent::run(this, &AcquisitionManager::readData);
		m_readFw->setFuture(f);
	}
}

void AcquisitionManager::readData()
{
	QMutexLocker locker(&m_mutex);
	if(m_tools["rms"] || m_tools["harmonics"] || m_tools["settings"]) {
		if(!m_processData.load()) {
			setProcessData(true);
		}
		m_attrHaveBeenRead = readPqmAttributes();
	}
	if(m_tools["waveform"]) {
		if(m_processData.load()) {
			setProcessData(false);
		}
		m_buffHaveBeenRead = readBufferedData();
	}
}

bool AcquisitionManager::readPqmAttributes()
{
	iio_device *dev = iio_context_find_device(m_ctx, DEVICE_PQM);
	if(!dev) {
		qDebug(CAT_PQM_ACQ) << "Device is unavailable!";
		return false;
	}
	int attrNo = iio_device_get_attrs_count(dev);
	int chnlsNo = iio_device_get_channels_count(dev);
	const char *attrName = nullptr;
	const char *chnlId = nullptr;
	char dest[MAX_ATTR_SIZE];
	for(int i = 0; i < attrNo; i++) {
		attrName = iio_device_get_attr(dev, i);
		iio_device_attr_read(dev, attrName, dest, MAX_ATTR_SIZE);
		m_pqmAttr[DEVICE_PQM][attrName] = QString(dest);
	}
	for(int i = 0; i < chnlsNo; i++) {
		iio_channel *chnl = iio_device_get_channel(dev, i);
		attrNo = iio_channel_get_attrs_count(chnl);
		chnlId = iio_channel_get_name(chnl);
		for(int j = 0; j < attrNo; j++) {
			attrName = iio_channel_get_attr(chnl, j);
			iio_channel_attr_read(chnl, attrName, dest, MAX_ATTR_SIZE);
			QString attrValue(dest);
			computeAdjustedAngle(attrName, attrValue);
			m_pqmAttr[chnlId][attrName] = attrValue;
			m_pqmLog->acquireAttrData(attrName, dest, chnlId);
		}
	}
	m_pqmLog->log();
	return true;
}

bool AcquisitionManager::readBufferedData()
{
	if(!m_buffer) {
		qWarning(CAT_PQM_ACQ) << "The buffer is NULL!";
		return false;
	}
	ssize_t ret = iio_buffer_refill(m_buffer);
	if(ret < 0) {
		qWarning(CAT_PQM_ACQ) << "An error occurred while refilling! [" << ret << "]";
		return false;
	}
	int samplesCounter = 0;
	int chnlIdx = 0;
	QString chnl;
	int16_t *startAdr = (int16_t *)iio_buffer_start(m_buffer);
	int16_t *endAdr = (int16_t *)iio_buffer_end(m_buffer);
	for(const QString &ch : qAsConst(m_chnlsName)) {
		m_bufferData[ch].clear();
		m_bufferData[ch] = {};
	}
	for(int16_t *ptr = startAdr; ptr != endAdr; ptr++) {
		chnlIdx = samplesCounter % m_chnlsName.size();
		chnl = m_chnlsName[chnlIdx];
		double d_ptr = convertFromHwToHost((int)*ptr, chnl);
		m_pqmLog->acquireBufferData(d_ptr, chnlIdx);
		m_bufferData[chnl].push_back(d_ptr);
		samplesCounter++;
	}
	m_pqmLog->log();
	return true;
}

void AcquisitionManager::onReadFinished()
{
	if(m_attrHaveBeenRead) {
		m_attrHaveBeenRead = false;
		Q_EMIT pqmAttrsAvailable(m_pqmAttr);
	}
	if(m_buffHaveBeenRead) {
		m_buffHaveBeenRead = false;
		Q_EMIT bufferDataAvailable(m_bufferData);
	}
	QMap<QString, bool>::const_iterator it = std::find(m_tools.cbegin(), m_tools.cend(), true);
	if(it != m_tools.cend()) {
		futureReadData();
	}
}

void AcquisitionManager::pingTimerTimeout()
{
	QMutexLocker locker(&m_mutex);
	m_pingTask->start();
	m_pingTask->wait(THREAD_FINISH_TIMEOUT);
}

double AcquisitionManager::convertFromHwToHost(int value, QString chnlId)
{
	bool okScale = false, okOffset = false;
	double scale = m_pqmAttr[chnlId]["scale"].toDouble(&okScale);
	double offset = m_pqmAttr[chnlId]["offset"].toDouble(&okOffset);
	double result = 0.0;
	if(okScale && okOffset) {
		result = (value + offset) * scale;
	}
	return result;
}

void AcquisitionManager::setConfigAttr(QMap<QString, QMap<QString, QString>> attr)
{
	if(!m_setFw->isRunning()) {
		QFuture<void> f = QtConcurrent::run(this, &AcquisitionManager::setData, attr);
		m_setFw->setFuture(f);
	}
}

void AcquisitionManager::startPing() { m_pingTimer->start(); }

void AcquisitionManager::stopPing() { m_pingTimer->stop(); }

void AcquisitionManager::setData(QMap<QString, QMap<QString, QString>> attr)
{
	QMutexLocker locker(&m_mutex);
	iio_device *dev = iio_context_find_device(m_ctx, DEVICE_PQM);
	if(!dev)
		return;
	const QStringList keys = attr[DEVICE_PQM].keys();
	for(const QString &key : keys) {
		if(m_pqmAttr[DEVICE_PQM].contains(key) &&
		   attr[DEVICE_PQM][key].compare(m_pqmAttr[DEVICE_PQM][key]) != 0) {
			QString newVal = attr[DEVICE_PQM][key];
			m_pqmAttr[DEVICE_PQM][key] = newVal;
			iio_device_attr_write(dev, key.toStdString().c_str(), newVal.toStdString().c_str());
		}
	}
}

void AcquisitionManager::setProcessData(bool en)
{
	iio_device *dev = iio_context_find_device(m_ctx, DEVICE_PQM);
	if(!dev) {
		qWarning(CAT_PQM_ACQ) << "Device is unavailable!";
		return;
	}
	int ret = iio_device_attr_write_bool(dev, "process_data", en);
	if(ret < 0) {
		qWarning(CAT_PQM_ACQ) << "Cannot write process_data attribute!";
		return;
	}
	m_processData.store(en);
	qInfo(CAT_PQM_ACQ) << "process_data was written successfully:" << en;
}

void AcquisitionManager::storeProcessData()
{
	QMutexLocker locker(&m_mutex);
	iio_device *dev = iio_context_find_device(m_ctx, DEVICE_PQM);
	if(!dev) {
		qWarning(CAT_PQM_ACQ) << "Device is unavailable!";
		return;
	}
	bool val = false;
	int ret = iio_device_attr_read_bool(dev, "process_data", &val);
	if(ret < 0) {
		qWarning(CAT_PQM_ACQ) << "Cannot read process_data attribute!";
	}
	m_processData.store(val);
}

void AcquisitionManager::computeAdjustedAngle(const QString &attrName, QString &angle)
{
	if(attrName.compare("angle") != 0) {
		return;
	}
	bool ok = false;
	double adjustedAngle = angle.toDouble(&ok);
	if(!ok) {
		return;
	}
	// The angle we receive from the device represents the delay, which is why the following operation needs to be
	// performed
	adjustedAngle = 360 - adjustedAngle;
	angle = QString::number(adjustedAngle);
}

bool AcquisitionManager::hasFwVers() const { return m_hasFwVers; }

#include "moc_acquisitionmanager.cpp"
