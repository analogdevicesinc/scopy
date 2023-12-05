#include "acquisitionmanager.h"
#include "qtconcurrentrun.h"
#include <QLoggingCategory>
#include <QTimer>

Q_LOGGING_CATEGORY(CAT_PQM_ACQ, "PqmAqcManager");
using namespace scopy::pqm;

AcquisitionManager::AcquisitionManager(iio_context *ctx, QObject *parent)
	: QObject(parent)
	, m_ctx(ctx)
	, m_buffer(nullptr)
{
	m_readFw = new QFutureWatcher<void>(this);
	m_setFw = new QFutureWatcher<void>(this);
	iio_device *dev = iio_context_find_device(m_ctx, DEVICE_PQM);
	//	iio_device *trigger = iio_context_find_device(m_ctx, "pqm-dev0");
	//	iio_device_set_trigger(dev, trigger);
	enableBufferChnls(dev);
	m_buffer = iio_device_create_buffer(dev, BUFFER_SIZE, false);
	if(!m_buffer) {
		qWarning(CAT_PQM_ACQ) << "Cannot create the buffer!";
	}

	m_dataRefreshTimer = new QTimer(this);
	m_dataRefreshTimer->setInterval(100);
	connect(m_dataRefreshTimer, &QTimer::timeout, this, &AcquisitionManager::futureReadData);
	connect(m_readFw, &QFutureWatcher<void>::finished, this, &AcquisitionManager::onReadFinished);
}

AcquisitionManager::~AcquisitionManager()
{
	if(m_dataRefreshTimer) {
		m_dataRefreshTimer->stop();
	}
	if(m_readFw->isRunning()) {
		m_readFw->waitForFinished();
	}
	if(m_ctx) {
		m_ctx = nullptr;
	}
	if(m_buffer) {
		iio_buffer_destroy(m_buffer);
		m_buffer = nullptr;
	}
	m_chnlsName.clear();
	m_bufferData.clear();
	m_pqmAttr.clear();
}

void AcquisitionManager::readPqmAttributes()
{
	iio_device *dev = iio_context_find_device(m_ctx, DEVICE_PQM);
	int attrNo = iio_device_get_attrs_count(dev);
	int chnlsNo = iio_device_get_channels_count(dev);
	const char *attrName = nullptr;
	const char *chnlId = nullptr;
	char *dest = new char[MAX_ATTR_SIZE];
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
			m_pqmAttr[chnlId][attrName] = QString(dest);
		}
	}
	delete[] dest;
	dest = nullptr;
}

void AcquisitionManager::readBufferedData()
{
	if(m_buffer) {
		ssize_t ret = iio_buffer_refill(m_buffer);
		if(ret > 0) {
			int i = 0;
			int idx = 0;
			QString chnl;
			float *startAdr = (float *)iio_buffer_start(m_buffer);
			float *endAdr = (float *)iio_buffer_end(m_buffer);
			for(const QString &ch : qAsConst(m_chnlsName)) {
				m_bufferData[ch].clear();
				m_bufferData[ch] = {};
			}
			for(float *ptr = startAdr; ptr != endAdr; ptr++) {
				idx = i % m_chnlsName.size();
				chnl = m_chnlsName[idx];
				float d_ptr = (float)*ptr;
				m_bufferData[chnl].push_back(d_ptr);
				i++;
			}
		}
	}
}

void AcquisitionManager::readData()
{
	readPqmAttributes();
	readBufferedData();
}

void AcquisitionManager::setData(QMap<QString, QMap<QString, QString>> attr)
{
	m_readFw->waitForFinished();
	m_readFw->pause();
	iio_device *dev = iio_context_find_device(m_ctx, DEVICE_PQM);
	if(!dev) {
		return;
	}
	const QStringList keys = attr[DEVICE_PQM].keys();
	for(const QString &key : keys) {
		if(m_pqmAttr[DEVICE_PQM].contains(key) &&
		   attr[DEVICE_PQM][key].compare(m_pqmAttr[DEVICE_PQM][key]) != 0) {
			QString newVal = attr[DEVICE_PQM][key];
			m_pqmAttr[DEVICE_PQM][key] = newVal;
			iio_device_attr_write(dev, key.toStdString().c_str(), newVal.toStdString().c_str());
		}
	}
	m_readFw->resume();
}

void AcquisitionManager::toolEnabled(bool en, QString toolName)
{
	m_tools[toolName] = en;
	QMap<QString, bool>::const_iterator it = std::find(m_tools.cbegin(), m_tools.cend(), true);
	if(it != m_tools.cend()) {
		if(!m_dataRefreshTimer->isActive()) {
			m_dataRefreshTimer->start();
		}
	} else {
		m_dataRefreshTimer->stop();
	}
}

void AcquisitionManager::setConfigAttr(QMap<QString, QMap<QString, QString>> attr)
{
	if(!m_setFw->isRunning()) {
		QFuture<void> f = QtConcurrent::run(this, &AcquisitionManager::setData, attr);
		m_setFw->setFuture(f);
	}
}

void AcquisitionManager::onReadFinished()
{
	Q_EMIT pqmAttrsAvailable(m_pqmAttr);
	Q_EMIT bufferDataAvailable(m_bufferData);
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
}

void AcquisitionManager::futureReadData()
{
	if(!m_readFw->isRunning()) {
		QFuture<void> f = QtConcurrent::run(this, &AcquisitionManager::readData);
		m_readFw->setFuture(f);
	}
}
