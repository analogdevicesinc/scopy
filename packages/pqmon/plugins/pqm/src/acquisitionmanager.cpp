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
#include <component/attribute.h>
#include <component/backends/iio/iioscanelement.h>
#include <component/channel.h>
#include <component/device.h>
#include <component/ping.h>
#include <component/streamview.h>
#include <pluginbase/preferences.h>

Q_LOGGING_CATEGORY(CAT_PQM_ACQ, "PqmAqcManager");
using namespace scopy::pqm;

AcquisitionManager::AcquisitionManager(component::ContextHandle ctx, QObject *parent)
	: QObject(parent)
	, m_ctx(std::move(ctx))
	, m_pqmLog(nullptr)
{
	Preferences *p = Preferences::GetInstance();
	m_concurrentAcq = p->get("pqm_concurrent").toBool();
	m_readFw = new QFutureWatcher<void>(this);
	m_setFw = new QFutureWatcher<void>(this);
	component::Device *dev = m_ctx->findChild<component::Device *>(DEVICE_PQM);
	if(dev) {
		// might need to set a trigger for the pqm device
		m_inputStream = dev->findChild<component::iio::IIOInputStream *>();
		m_pqmLog = new PqmDataLogger(this);
		m_hasFwVers = dev->findChild<component::Attribute *>(
			"fw_version", Qt::FindDirectChildrenOnly); // iio_device_find_attr(dev, "fw_version");
		readPqmAttributes();
		QList<int> enabledChnls = enableBufferChnls(dev);

		if(!QCoro::waitFor(m_inputStream->openAsync({enabledChnls, BUFFER_SIZE}))) {
			qWarning(CAT_PQM_ACQ) << "Cannot create the buffer!";
		}
		connect(m_readFw, &QFutureWatcher<void>::finished, this, &AcquisitionManager::onReadFinished,
			Qt::QueuedConnection);
		connect(this, &AcquisitionManager::logData, m_pqmLog, &PqmDataLogger::logPressed);
		connect(p, &Preferences::preferenceChanged, this, [this](QString pref, QVariant value) {
			if(pref == "pqm_concurrent") {
				m_concurrentAcq = value.toBool();
				m_alternateExecution = !m_concurrentAcq;
			}
		});
	} else {
		qWarning(CAT_PQM_ACQ) << "The PQM device is not available!";
	}
}

AcquisitionManager::~AcquisitionManager()
{
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
	if(m_inputStream && m_inputStream->isOpen()) {
		QCoro::waitFor(m_inputStream->closeAsync());
		m_inputStream = nullptr;
	}
	m_buffChnls.clear();
	m_bufferData.clear();
	m_pqmAttr.clear();
}

QList<int> AcquisitionManager::enableBufferChnls(component::Device *dev)
{
	QList<int> enabledChnls{};
	const QList<component::Channel *> chnls = dev->findChildren<component::Channel *>(Qt::FindDirectChildrenOnly);
	for(component::Channel *chnl : chnls) {
		if(chnl->isOutput()) {
			m_eventsChnls.push_back(chnl->name());
			continue;
		}
		m_buffChnls.push_back(chnl->name());
	}
	const QList<component::iio::IIOScanElement *> elements =
		m_inputStream->findChildren<component::iio::IIOScanElement *>(Qt::FindDirectChildrenOnly);
	for(component::iio::IIOScanElement *el : elements) {
		enabledChnls.push_back(el->index());
	}
	m_pqmLog->setChnlsName(m_buffChnls);
	return enabledChnls;
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
		QFuture<void> f = QtConcurrent::run(&AcquisitionManager::readData, this);
		m_readFw->setFuture(f);
	}
}

void AcquisitionManager::readData()
{
	QMutexLocker locker(&m_mutex);

	bool needsAttrData = m_tools["rms"] || m_tools["harmonics"] || m_tools["settings"];
	bool needsBufferData = m_tools["waveform"];
	if(m_concurrentAcq && needsAttrData && needsBufferData) {
		if(m_alternateExecution) {
			readBuffData();
		} else {
			readAttrData();
		}
		m_alternateExecution = !m_alternateExecution;
	} else {
		if(needsAttrData) {
			readAttrData();
		}
		if(needsBufferData) {
			readBuffData();
		}
	}
}

void AcquisitionManager::readAttrData()
{
	if(!m_processData.load()) {
		setProcessData(true);
	}
	m_attrHaveBeenRead = readPqmAttributes();
	adjustMap("angle", &AcquisitionManager::computeAdjustedAngle);
}

void AcquisitionManager::readBuffData()
{
	if(m_processData.load()) {
		setProcessData(false);
	}
	m_buffHaveBeenRead = readBufferedData();
}

bool AcquisitionManager::readPqmAttributes()
{
	component::Device *dev = m_ctx->findChild<component::Device *>(DEVICE_PQM);
	if(!dev) {
		qDebug(CAT_PQM_ACQ) << "Device is unavailable!";
		return false;
	}
	const QList<component::Attribute *> devAttributes =
		dev->findChildren<component::Attribute *>(Qt::FindDirectChildrenOnly);
	for(component::Attribute *attr : devAttributes) {
		if(attr->readCapability()) {
			QCoro::waitFor(attr->readCapability()->readAsync());
		}
		m_pqmAttr[DEVICE_PQM][attr->name()] = attr->cachedValue();
	}
	const QList<component::Channel *> chnls = dev->findChildren<component::Channel *>(Qt::FindDirectChildrenOnly);
	for(component::Channel *chnl : chnls) {
		const QList<component::Attribute *> chAttributes =
			chnl->findChildren<component::Attribute *>(Qt::FindDirectChildrenOnly);
		for(component::Attribute *attr : chAttributes) {
			if(attr->readCapability()) {
				QCoro::waitFor(attr->readCapability()->readAsync());
			}
			m_pqmAttr[chnl->name()][attr->name()] = attr->cachedValue();
		}
	}
	m_pqmLog->acquireAttrData(m_pqmAttr);
	handlePQEvents();
	m_pqmLog->log();
	return true;
}

bool AcquisitionManager::readBufferedData()
{
	if(!m_inputStream || !m_inputStream->isOpen()) {
		qWarning(CAT_PQM_ACQ) << "The buffer is NULL!";
		return false;
	}
	Result<void> r = QCoro::waitFor(m_inputStream->refillAsync());
	if(!r) {
		qWarning(CAT_PQM_ACQ) << "An error occurred while refilling! [" << r.error().errorCode() << "]";
		return false;
	}

	component::StreamView view(m_inputStream->readFormat());
	for(const QString &ch : std::as_const(m_buffChnls)) {
		m_bufferData[ch].clear();
		m_bufferData[ch] = {};
	}
	int chnlIdx = 0;
	for(const QVector<double> &chnlStream : view.toDoubles()) {
		const QString &chnl = m_buffChnls[chnlIdx];
		m_bufferData[chnl].append(chnlStream);
		chnlIdx++;
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
	if(it != m_tools.cend() && m_readFw) {
		futureReadData();
	}
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
		QFuture<void> f = QtConcurrent::run(&AcquisitionManager::setData, this, attr);
		m_setFw->setFuture(f);
	}
}

void AcquisitionManager::startPing()
{
	component::Ping *ping = m_ctx->findChild<component::Ping *>();
	ping->startMonitoring(2000);
}

void AcquisitionManager::stopPing()
{
	component::Ping *ping = m_ctx->findChild<component::Ping *>();
	ping->stopMonitoring();
}

void AcquisitionManager::setData(QMap<QString, QMap<QString, QString>> attr)
{
	QMutexLocker locker(&m_mutex);
	component::Device *dev = m_ctx->findChild<component::Device *>(DEVICE_PQM);
	if(!dev) {
		return;
	}
	const QStringList keys = attr[DEVICE_PQM].keys();
	for(const QString &key : keys) {
		if(m_pqmAttr[DEVICE_PQM].contains(key) &&
		   attr[DEVICE_PQM][key].compare(m_pqmAttr[DEVICE_PQM][key]) != 0) {
			QString newVal = attr[DEVICE_PQM][key];
			m_pqmAttr[DEVICE_PQM][key] = newVal;
			component::Attribute *attr = dev->findChild<component::Attribute *>(key);
			if(attr && attr->writeCapability()) {
				QCoro::waitFor(attr->writeCapability()->writeAsync(newVal));
			}
		}
	}
}

void AcquisitionManager::setProcessData(bool en)
{
	component::Device *dev = m_ctx->findChild<component::Device *>(DEVICE_PQM);
	if(!dev) {
		qWarning(CAT_PQM_ACQ) << "Device is unavailable!";
		return;
	}
	component::Attribute *attr = dev->findChild<component::Attribute *>("process_data");
	if(!attr || !attr->writeCapability() ||
	   !QCoro::waitFor(attr->writeCapability()->writeAsync(QString::number(en)))) {
		qWarning(CAT_PQM_ACQ) << "Cannot write process_data attribute!";
		return;
	}

	m_processData.store(en);
	qInfo(CAT_PQM_ACQ) << "process_data was written successfully:" << en;
}

void AcquisitionManager::storeProcessData()
{
	QMutexLocker locker(&m_mutex);
	component::Device *dev = m_ctx->findChild<component::Device *>(DEVICE_PQM);
	if(!dev) {
		qWarning(CAT_PQM_ACQ) << "Device is unavailable!";
		return;
	}
	component::Attribute *attr = dev->findChild<component::Attribute *>("process_data");
	Result<QByteArray> r = QCoro::waitFor(attr->readCapability()->readAsync());
	if(!r) {
		qWarning(CAT_PQM_ACQ) << "Cannot read process_data attribute!";
	} else {
		m_processData.store(r.value().toInt());
	}
}

void AcquisitionManager::handlePQEvents()
{
	QString logMsg = "";
	for(const QString &ch : std::as_const(m_eventsChnls)) {
		if(m_pqmAttr[ch]["countEvent"].toInt() == 0) {
			continue;
		}
		logMsg.append(ch + ",");
		for(auto it = m_pqmAttr[ch].begin(); it != m_pqmAttr[ch].end(); ++it) {
			logMsg.append(it.key() + "," + it.value() + ",");
		}
		logMsg.append("\n,");
	}
	if(!logMsg.isEmpty()) {
		m_pqmLog->acquirePqEvents(logMsg);
		Q_EMIT pqEvent();
	}
}

void AcquisitionManager::computeAdjustedAngle(QString &angle)
{
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

void AcquisitionManager::adjustMap(const QString &attr, std::function<void(QString &value)> adjuster)
{
	for(auto it = m_pqmAttr.begin(); it != m_pqmAttr.end(); ++it) {
		if(!it.value().contains(attr)) {
			continue;
		}
		adjuster(it.value()[attr]);
	}
}

bool AcquisitionManager::hasFwVers() const { return m_hasFwVers; }

QString AcquisitionManager::getLogFilePath() const { return m_pqmLog ? m_pqmLog->getFilePath() : QString(); }

#include "moc_acquisitionmanager.cpp"
